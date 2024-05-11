#include "compiler.h"
#include "chunk.h"
#include "object.h"
#include "scanner.h"

#include <array>
#include <cassert>
#include <format>
#include <iostream>
#include <string>

#define UNINITIALIZED -1

namespace CompilerImpl
{
	using namespace Lox;

	struct Parser
	{
		Token current;
		Token previous;
		bool had_error = false;
		bool panic_mode = false;
	};

	enum class Precedence : u8
	{
		NONE,
		ASSIGNMENT,	   // =
		OR,			   // or
		AND,		   // and
		EQUALITY,	   // == !=
		COMPARISON,	   // < > <= >=
		TERM,		   // + -
		FACTOR,		   // * /
		UNARY,		   // ! -
		CALL,		   // . ()
		PRIMARY
	};

	using ParseFn = void (*)(bool can_assign);

	struct ParseRule
	{
		ParseFn prefix;
		ParseFn infix;
		Precedence precedence;
	};

	struct Local
	{
		Token name;
		i32 depth = UNINITIALIZED;
	};

	struct Upvalue
	{
		u8 index;		  // Which local slot is captured
		bool is_local;	  // True if it's a local variable of the surrounding function; False if it's itself an upvalue
	};

	enum class FunctionType : u8
	{
		FUNCTION,	 // Compiling a function body
		SCRIPT		 // Compiling top-level code
	};

	struct Compiler
	{
		Compiler* enclosing = nullptr;
		ObjectFunction* function = nullptr;
		FunctionType type;

		std::array<Local, UINT8_MAX + 1> locals;
		i32 local_count = 0;
		Upvalue upvalues[UINT8_MAX + 1];
		i32 scope_depth = 0;
	};

	Parser parser;
	Chunk* compiling_chunk;
	Compiler* current_compiler = nullptr;

	const ParseRule* get_rule(TokenType type);
	void expression();
	void statement();
	void declaration();
	void var_declaration();
	void and_(bool can_assign);
	void or_(bool can_assign);
	void parse_precedence(Precedence prec);
	void expression_statement();
	void begin_scope();
	void end_scope();
	u8 parse_variable(const char* error_message);
	void define_variable(u8 global_index);

	void init_compiler(Compiler* compiler, FunctionType type)
	{
		compiler->enclosing = current_compiler;
		compiler->type = type;
		compiler->local_count = 0;
		compiler->scope_depth = 0;
		compiler->function = ObjectFunction::allocate();
		current_compiler = compiler;

		if (type != FunctionType::SCRIPT)
		{
			current_compiler->function->name = Lox::ObjectString::allocate(	   //
				std::string{parser.previous.start, (size_t)parser.previous.length}
			);
		}

		// The compiler implicitly claims stack slot zero for the VM's internal use
		Local* local = &current_compiler->locals[current_compiler->local_count++];
		local->depth = 0;
		local->name.start = "";
		local->name.length = 0;
	}

	void error_at(Token* token, const char* message)
	{
		if (parser.panic_mode)
		{
			return;
		}
		parser.panic_mode = true;

		std::cerr << std::format("[line {}] Error", token->line);

		if (token->type == TokenType::EOF_)
		{
			std::cerr << " at end";
		}
		else if (token->type == TokenType::ERROR)
		{
			// Nothing
		}
		else
		{
			std::cerr << std::format(" at '{:.{}}'", token->start, token->length);
		}

		std::cerr << std::format(": {}", message) << std::endl;
		parser.had_error = true;
	}

	void error_at_current(const char* message)
	{
		error_at(&parser.current, message);
	}

	void error(const char* message)
	{
		error_at(&parser.current, message);
	}

	void advance()
	{
		parser.previous = parser.current;

		while (true)
		{
			parser.current = scan_token();
			if (parser.current.type != TokenType::ERROR)
			{
				break;
			}

			error_at_current(parser.current.start);
		}
	}

	bool check(TokenType type)
	{
		return parser.current.type == type;
	}

	bool match(TokenType type)
	{
		if (!check(type))
		{
			return false;
		}

		advance();
		return true;
	}

	void consume(TokenType type, const char* message)
	{
		if (parser.current.type == type)
		{
			advance();
			return;
		}

		error_at_current(message);
	}

	Chunk* current_chunk()
	{
		return &current_compiler->function->chunk;
	}

	u8 make_constant(Value value)
	{
		int index = current_chunk()->add_constant(value);
		if (index > UINT8_MAX)
		{
			error("Too many constants in one chunk");
			return 0;
		}

		return (u8)index;
	}

	u8 identifier_constant(const Token& name)
	{
		Lox::ObjectString* new_str = Lox::ObjectString::allocate(std::string{name.start, (size_t)name.length});
		return make_constant(new_str);
	}

	bool identifiers_equal(const Token& a, const Token& b)
	{
		if (a.length != b.length)
		{
			return false;
		}

		return std::memcmp(a.start, b.start, a.length) == 0;
	}

	i32 resolve_local(Compiler* compiler, const Token& name)
	{
		// Note: Locals and the location of the local values within the actual stack
		// match exactly, as temporary values never persist between locals (i.e. the
		// locals are all packed on the lowest significant positions of the stack).
		// This because temporaries either "become a local" or they are popped at the
		// end of an expression statement
		for (i32 i = compiler->local_count - 1; i >= 0; i--)
		{
			Local* local = &compiler->locals[i];
			if (identifiers_equal(name, local->name))
			{
				if (local->depth == UNINITIALIZED)
				{
					error("Can't read local variable in its own initializer");
				}

				return i;
			}
		}

		return -1;
	}

	i32 add_upvalue(Compiler* compiler, u8 index, bool is_local)
	{
		i32 upvalue_count = compiler->function->upvalue_count;

		// Reuse upvalue for the same slot index if possible
		for (i32 i = 0; i < upvalue_count; ++i)
		{
			Upvalue* upvalue = &compiler->upvalues[i];
			if (upvalue->index == index && upvalue->is_local == is_local)
			{
				return i;
			}
		}

		if (upvalue_count == UINT8_MAX + 1)
		{
			error("Too many closure variables in function");
			return 0;
		}

		compiler->upvalues[upvalue_count].is_local = is_local;
		compiler->upvalues[upvalue_count].index = index;
		return compiler->function->upvalue_count++;
	}

	i32 resolve_upvalue(Compiler* compiler, const Token& name)
	{
		if (compiler->enclosing == nullptr)
		{
			return -1;
		}

		i32 local = resolve_local(compiler->enclosing, name);
		if (local != -1)
		{
			return add_upvalue(compiler, (u8)local, true);
		}

		i32 upvalue = resolve_upvalue(compiler->enclosing, name);
		if (upvalue != -1)
		{
			return add_upvalue(compiler, (u8)upvalue, false);
		}

		return -1;
	}

	void emit_byte(u8 byte)
	{
		current_chunk()->write_chunk(byte, parser.previous.line);
	}

	void emit_bytes(u8 b1, u8 b2)
	{
		emit_byte(b1);
		emit_byte(b2);
	}

	i32 emit_jump(Op instruction)
	{
		emit_byte((u8)instruction);
		emit_byte(0xFF);
		emit_byte(0xFF);
		return static_cast<i32>(current_chunk()->code.size() - 2);
	}

	void emit_loop(i32 loop_start)
	{
		emit_byte((u8)Op::LOOP);

		i32 offset = (i32)(current_chunk()->code.size() - loop_start + 2);
		if (offset > UINT16_MAX)
		{
			error("Loop body too large");
		}

		emit_byte((offset >> 8) & 0xFF);
		emit_byte(offset & 0xFF);
	}

	void emit_return()
	{
		emit_byte((u8)Op::NIL);
		emit_byte((u8)Op::RETURN);
	}

	void emit_constant(Value value)
	{
		emit_bytes((u8)Op::CONSTANT, make_constant(value));
	}

	void patch_jump(i32 offset)
	{
		// -2 to adjust for the bytecode for the jump offset itself
		i32 distance = (i32)(current_chunk()->code.size() - offset - 2);

		if (distance > UINT16_MAX)
		{
			error("Too much code to jump over");
		}

		current_chunk()->code[offset + 0] = (distance >> 8) & 0xFF;
		current_chunk()->code[offset + 1] = (distance >> 0) & 0xFF;
	}

	ObjectFunction* end_compiler()
	{
		emit_return();
		ObjectFunction* function = current_compiler->function;

#if DEBUG_PRINT_CODE
		if (!parser.had_error)
		{
			current_chunk()->disassemble_chunk(function->name != nullptr ? function->name->get_string().c_str() : "<script>");
		}
#endif

		current_compiler = current_compiler->enclosing;
		return function;
	}

	void number([[maybe_unused]] bool can_assign)
	{
		double value = strtod(parser.previous.start, nullptr);
		emit_constant(value);
	}

	void expression()
	{
		parse_precedence(Precedence::ASSIGNMENT);
	}

	void unary([[maybe_unused]] bool can_assign)
	{
		TokenType op_type = parser.previous.type;

		// Compile the operand
		parse_precedence(Precedence::UNARY);

		// Emit the operator instruction
		switch (op_type)
		{
			case TokenType::BANG:
			{
				emit_byte((u8)Op::NOT);
				break;
			}
			case TokenType::MINUS:
			{
				emit_byte((u8)Op::NEGATE);
				break;
			}
			default:
			{
				assert(false);
				return;
			}
		}
	}

	void binary([[maybe_unused]] bool can_assign)
	{
		TokenType op_type = parser.previous.type;

		// Compile right operand
		// +1 precedence value because these are left associative, so we don't want to keep on parsing
		// the same operator. Example: We want (((1 + 2) + 3) + 4), and not (1 + (2 + (3 + 4)))
		const ParseRule* rule = get_rule(op_type);
		parse_precedence((Precedence)((u8)rule->precedence + 1));

		switch (op_type)
		{
			case TokenType::PLUS:
			{
				emit_byte((u8)Op::ADD);
				break;
			}
			case TokenType::MINUS:
			{
				emit_byte((u8)Op::SUBTRACT);
				break;
			}
			case TokenType::STAR:
			{
				emit_byte((u8)Op::MULTIPLY);
				break;
			}
			case TokenType::SLASH:
			{
				emit_byte((u8)Op::DIVIDE);
				break;
			}
			case TokenType::BANG_EQUAL:
			{
				emit_bytes((u8)Op::EQUAL, (u8)Op::NOT);
				break;
			}
			case TokenType::EQUAL_EQUAL:
			{
				emit_byte((u8)Op::EQUAL);
				break;
			}
			case TokenType::GREATER:
			{
				emit_byte((u8)Op::GREATER);
				break;
			}
			case TokenType::GREATER_EQUAL:
			{
				emit_bytes((u8)Op::LESS, (u8)Op::NOT);
				break;
			}
			case TokenType::LESS:
			{
				emit_byte((u8)Op::LESS);
				break;
			}
			case TokenType::LESS_EQUAL:
			{
				emit_bytes((u8)Op::GREATER, (u8)Op::NOT);
				break;
			}
			default:
			{
				assert(false);
				return;
			}
		}
	}

	u8 argument_list()
	{
		u8 arg_count = 0;
		if (!check(TokenType::RIGHT_PAREN))
		{
			do
			{
				expression();
				if (arg_count == 255)
				{
					error("Can't have more than 255 arguments");
				}
				arg_count++;
			} while (match(TokenType::COMMA));
		}
		consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
		return arg_count;
	}

	void call([[maybe_unused]] bool can_assign)
	{
		u8 arg_count = argument_list();
		emit_bytes((u8)Op::CALL, arg_count);
	}

	void grouping([[maybe_unused]] bool can_assign)
	{
		expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
	}

	void literal([[maybe_unused]] bool can_assign)
	{
		// Since parse_precedence already consumed the keyword token itself, we just need
		// to output the instruction
		switch (parser.previous.type)
		{
			case TokenType::FALSE:
			{
				emit_byte((u8)Op::FALSE);
				break;
			}
			case TokenType::TRUE:
			{
				emit_byte((u8)Op::TRUE);
				break;
			}
			case TokenType::NIL:
			{
				emit_byte((u8)Op::NIL);
				break;
			}
			default:
			{
				assert(false);
				return;
			}
		}
	}

	void string([[maybe_unused]] bool can_assign)
	{
		Lox::ObjectString* new_str = Lox::ObjectString::allocate(	 //
			std::string{parser.previous.start + 1, (size_t)(parser.previous.length - 2)}
		);

		emit_constant(new_str);
	}

	void named_variable(const Token& name, bool can_assign)
	{
		Op get_op;
		Op set_op;
		i32 op_arg = resolve_local(current_compiler, name);
		if (op_arg != -1)
		{
			get_op = Op::GET_LOCAL;
			set_op = Op::SET_LOCAL;
		}
		else if ((op_arg = resolve_upvalue(current_compiler, name)) != -1)
		{
			get_op = Op::GET_UPVALUE;
			set_op = Op::SET_UPVALUE;
		}
		else
		{
			op_arg = identifier_constant(name);
			get_op = Op::GET_GLOBAL;
			set_op = Op::SET_GLOBAL;
		}

		// We may be parsing something like `menu.brunch(sunday).beverage = "mimosa";`, where
		// the left-hand side of the equal signs could have been parsed as a get expression, up to
		// the point where we run into the '=' and realize it's a setter instead
		if (can_assign && match(TokenType::EQUAL))
		{
			expression();
			emit_bytes((u8)set_op, (u8)op_arg);
		}
		else
		{
			emit_bytes((u8)get_op, (u8)op_arg);
		}
	}

	void variable(bool can_assign)
	{
		named_variable(parser.previous, can_assign);
	}

	// Will parse all expressions at 'prec' level or higher (higher value, so CALL > UNARY)
	void parse_precedence(Precedence prec)
	{
		// Consume the first token
		advance();

		// Parse the first token: That should always be something valid - a number, a 'var', an
		// identifier, etc.
		//
		// prefix_rule may end up consuming more tokens to fill in the "left-hand-side expression",
		// if we're talking about a binary operator for example.
		//
		// prefix_rule may end up recursing back into parse_precedence, and will parse everything
		// it is allowed to given it's current precedence level, and only later give us back
		// control, so we can parse the rest of the expression above its precedence, if there is
		// any
		ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
		if (prefix_rule == nullptr)
		{
			error("Expected expression");
			return;
		}

		bool can_assign = prec <= Precedence::ASSIGNMENT;
		prefix_rule(can_assign);

		// While the next token has higher or equal precedence than the level we're allowed
		// to parse, continue parsing stuff
		while (get_rule(parser.current.type)->precedence >= prec)
		{
			// Consume the next token
			advance();

			// Parse the next token as an infix operator: This will be a "+", a "/", etc.
			// Calling infix_rule() here may consume many more tokens, to parse the entire
			// expression
			//
			// After consuming those tokens, we'll do another pass of the while loop and
			// try consuming the rest of the expression with the same precedence level. If
			// that doesn't work, we'll return back to our caller
			//
			// Note that when we call infix_rule, we're essentially providing "what we parsed so far"
			// as its left operand. Remember we're already emitting bytecode as we parse these, so
			// the bytes for the left-hand-side were *already emitted* at that point
			ParseFn infix_rule = get_rule(parser.previous.type)->infix;
			infix_rule(can_assign);
		}

		if (can_assign && match(TokenType::EQUAL))
		{
			error("Invalid assignment target");
		}
	}

	static constexpr std::array<ParseRule, (u8)TokenType::NUM> rules = []()
	{
		std::array<ParseRule, (u8)TokenType::NUM> result;

		// clang-format off
		//                                      prefix   infix    infix precedence
		result[(u8)TokenType::LEFT_PAREN]    = {grouping, call,  	Precedence::CALL};
		result[(u8)TokenType::RIGHT_PAREN]   = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::LEFT_BRACE]    = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::RIGHT_BRACE]   = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::COMMA]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::DOT]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::MINUS]         = {unary,    binary,   Precedence::TERM};
		result[(u8)TokenType::PLUS]          = {nullptr,  binary,   Precedence::TERM};
		result[(u8)TokenType::SEMICOLON]     = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::SLASH]         = {nullptr,  binary,   Precedence::FACTOR};
		result[(u8)TokenType::STAR]          = {nullptr,  binary,   Precedence::FACTOR};
		result[(u8)TokenType::BANG]          = {unary,    nullptr,  Precedence::NONE};
		result[(u8)TokenType::BANG_EQUAL]    = {nullptr,  binary,   Precedence::EQUALITY};
		result[(u8)TokenType::EQUAL]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::EQUAL_EQUAL]   = {nullptr,  binary,   Precedence::EQUALITY};
		result[(u8)TokenType::GREATER]       = {nullptr,  binary,   Precedence::COMPARISON};
		result[(u8)TokenType::GREATER_EQUAL] = {nullptr,  binary,   Precedence::COMPARISON};
		result[(u8)TokenType::LESS]          = {nullptr,  binary,   Precedence::COMPARISON};
		result[(u8)TokenType::LESS_EQUAL]    = {nullptr,  binary,   Precedence::COMPARISON};
		result[(u8)TokenType::IDENTIFIER]    = {variable, nullptr,  Precedence::NONE};
		result[(u8)TokenType::STRING]        = {string,   nullptr,  Precedence::NONE};
		result[(u8)TokenType::NUMBER]        = {number,   nullptr,  Precedence::NONE};
		result[(u8)TokenType::AND]           = {nullptr,  and_,  	Precedence::AND};
		result[(u8)TokenType::CLASS]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::ELSE]          = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FALSE]         = {literal,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FOR]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FUN]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::IF]            = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::NIL]           = {literal,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::OR]            = {nullptr,  or_,  	Precedence::OR};
		result[(u8)TokenType::PRINT]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::RETURN]        = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::SUPER]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::THIS]          = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::TRUE]          = {literal,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::VAR]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::WHILE]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::ERROR]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::EOF_]          = {nullptr,  nullptr,  Precedence::NONE};
		// clang-format on

		return result;
	}();

	const ParseRule* get_rule(TokenType type)
	{
		return &rules[(u8)type];
	}

	void synchronize()
	{
		parser.panic_mode = false;

		while (parser.current.type != TokenType::EOF_)
		{
			// Detects the end of an expression statement
			if (parser.previous.type == TokenType::SEMICOLON)
			{
				return;
			}

			// Detects the end of the other statement types (or rather,
			// the start of the next statement)
			switch (parser.current.type)
			{
				case TokenType::CLASS:
				case TokenType::FUN:
				case TokenType::VAR:
				case TokenType::FOR:
				case TokenType::IF:
				case TokenType::WHILE:
				case TokenType::PRINT:
				case TokenType::RETURN:
				{
					return;
				}
				default:
				{
					// Do nothing
				}
			}

			advance();
		}
	}

	void print_statement()
	{
		expression();
		consume(TokenType::SEMICOLON, "Expected ';' after value");
		emit_byte((u8)Op::PRINT);
	}

	void for_statement()
	{
		begin_scope();

		// Initializer
		consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");
		if (match(TokenType::SEMICOLON))
		{
			// No initializer
		}
		else if (match(TokenType::VAR))
		{
			var_declaration();
		}
		else
		{
			// Use this instead of `expression` because it looks for the semicolon and pops the result off the stack on its own
			expression_statement();
		}

		// Condition
		i32 loop_start = (i32)(current_chunk()->code.size());
		i32 exit_jump = -1;
		if (!match(TokenType::SEMICOLON))
		{
			expression();
			consume(TokenType::SEMICOLON, "Expected ';' after loop condition");

			// Jump out of the loop if the condition is false
			exit_jump = emit_jump(Op::JUMP_IF_FALSE);
			emit_byte((u8)Op::POP);	   // Pop the condition expression result
		}

		// Increment
		if (!match(TokenType::RIGHT_PAREN))
		{
			// We'll jump into the body first, but re-wire things:
			//
			// After the body `statement`, the `emit_loop(loop_start)` will take us to increment_start
			// and run the increment expression again, then pop it off the stack and using the
			// `emit_loop(loop_start)` get us back to before the condition block again
			i32 body_jump = emit_jump(Op::JUMP);
			i32 increment_start = (i32)(current_chunk()->code.size());

			expression();
			emit_byte((u8)Op::POP);
			consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses");

			emit_loop(loop_start);
			loop_start = increment_start;
			patch_jump(body_jump);
		}

		statement();
		emit_loop(loop_start);

		if (exit_jump != -1)
		{
			patch_jump(exit_jump);
			emit_byte((u8)Op::POP);	   // Pop the condition expression result
		}

		end_scope();
	}

	void while_statement()
	{
		i32 loop_start = (i32)current_chunk()->code.size();

		consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
		expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");

		i32 exit_jump = emit_jump(Op::JUMP_IF_FALSE);
		emit_byte((u8)Op::POP);
		statement();
		emit_loop(loop_start);

		patch_jump(exit_jump);
		emit_byte((u8)Op::POP);
	}

	void expression_statement()
	{
		expression();
		consume(TokenType::SEMICOLON, "Expected ';' after value");
		emit_byte((u8)Op::POP);
	}

	void if_statement()
	{
		consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
		expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");

		i32 then_jump = emit_jump(Op::JUMP_IF_FALSE);
		emit_byte((u8)Op::POP);	   // Pop the result of the condition expression off the stack as a statement must have zero stack effect
		statement();
		i32 else_jump = emit_jump(Op::JUMP);

		patch_jump(then_jump);	   // Skip the 'then statement' to here if the condition didn't pass
		emit_byte((u8)Op::POP);	   // Pop the result of the condition expression off the stack as a statement must have zero stack effect
		if (match(TokenType::ELSE))
		{
			statement();
		}
		patch_jump(else_jump);	  // Skip the 'else statement' to here if the condition did pass
	}

	void return_statement()
	{
		if (current_compiler->type == FunctionType::SCRIPT)
		{
			error("Can't return from top-level code");
		}

		if (match(TokenType::SEMICOLON))
		{
			emit_return();
		}
		else
		{
			expression();
			consume(TokenType::SEMICOLON, "Expected ';' after return value");
			emit_byte((u8)Op::RETURN);
		}
	}

	void block()
	{
		while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::EOF_))
		{
			declaration();
		}

		consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
	}

	void function(FunctionType type)
	{
		Compiler compiler;
		init_compiler(&compiler, type);
		begin_scope();

		consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
		if (!check(TokenType::RIGHT_PAREN))
		{
			do
			{
				current_compiler->function->arity++;
				if (current_compiler->function->arity > 255)
				{
					error_at_current("Can't have more than 255 parameters");
				}

				u8 constant = parse_variable("Expected parameter name");
				define_variable(constant);
			} while (match(TokenType::COMMA));
		}

		consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
		consume(TokenType::LEFT_BRACE, "Expected '{' before function body");

		block();

		ObjectFunction* function = end_compiler();
		emit_bytes((u8)Op::CLOSURE, make_constant(function));

		for (i32 i = 0; i < function->upvalue_count; ++i)
		{
			emit_byte(compiler.upvalues[i].is_local ? 1 : 0);
			emit_byte(compiler.upvalues[i].index);
		}
	}

	void begin_scope()
	{
		current_compiler->scope_depth++;
	}

	void end_scope()
	{
		current_compiler->scope_depth--;

		while (current_compiler->local_count > 0 && current_compiler->locals[current_compiler->local_count - 1].depth > current_compiler->scope_depth)
		{
			emit_byte((u8)Op::POP);
			current_compiler->local_count--;
		}
	}

	void statement()
	{
		if (match(TokenType::PRINT))
		{
			print_statement();
		}
		else if (match(TokenType::IF))
		{
			if_statement();
		}
		else if (match(TokenType::RETURN))
		{
			return_statement();
		}
		else if (match(TokenType::FOR))
		{
			for_statement();
		}
		else if (match(TokenType::WHILE))
		{
			while_statement();
		}
		else if (match(TokenType::LEFT_BRACE))
		{
			begin_scope();
			block();
			end_scope();
		}
		else
		{
			expression_statement();
		}
	}

	void add_local(const Token& var_name)
	{
		if (current_compiler->local_count == UINT8_MAX + 1)
		{
			error("Too many local variables");
			return;
		}

		Local* local = &current_compiler->locals[current_compiler->local_count++];
		local->name = var_name;
		local->depth = current_compiler->scope_depth;
	}

	void mark_initialized()
	{
		// If we're calling this when compiling something on top-level (like a top-level function
		// declaration), there is no local variable so we shouldn't do anything here
		if (current_compiler->scope_depth == 0)
		{
			return;
		}

		current_compiler->locals[current_compiler->local_count - 1].depth = current_compiler->scope_depth;
	}

	void declare_variable()
	{
		if (current_compiler->scope_depth == 0)
		{
			return;
		}

		const Token& var_name = parser.previous;
		for (i32 i = current_compiler->local_count - 1; i >= 0; i--)
		{
			Local* local = &current_compiler->locals[i];

			if (local->depth != -1 && local->depth < current_compiler->scope_depth)
			{
				break;
			}

			if (identifiers_equal(var_name, local->name))
			{
				error("A variable with this name already exists in this scope");
			}
		}

		add_local(var_name);
	}

	void define_variable(u8 global_index)
	{
		// Don't need to do anything at runtime: The temporary for the variable's value
		// is already in the stack anyway
		if (current_compiler->scope_depth > 0)
		{
			mark_initialized();
			return;
		}

		emit_bytes((u8)Op::DEFINE_GLOBAL, global_index);
	}

	void and_([[maybe_unused]] bool can_assign)
	{
		i32 end_jump = emit_jump(Op::JUMP_IF_FALSE);

		emit_byte((u8)Op::POP);
		parse_precedence(Precedence::AND);

		patch_jump(end_jump);
	}

	void or_([[maybe_unused]] bool can_assign)
	{
		i32 else_jump = emit_jump(Op::JUMP_IF_FALSE);
		i32 end_jump = emit_jump(Op::JUMP);

		patch_jump(else_jump);
		emit_byte((u8)Op::POP);

		parse_precedence(Precedence::OR);
		patch_jump(end_jump);
	}

	u8 parse_variable(const char* error_message)
	{
		consume(TokenType::IDENTIFIER, error_message);

		declare_variable();
		if (current_compiler->scope_depth > 0)
		{
			// "At runtime, locals aren’t looked up by name. There’s no need to stuff the variable’s name into
			// the constant table, so if the declaration is inside a local scope, we return a dummy table index instead."
			return 0;
		}

		return identifier_constant(parser.previous);
	}

	void fun_declaration()
	{
		u8 global = parse_variable("Expected function name");
		mark_initialized();
		function(FunctionType::FUNCTION);
		define_variable(global);
	}

	void var_declaration()
	{
		u8 global_index = parse_variable("Expected variable name");

		if (match(TokenType::EQUAL))
		{
			expression();
		}
		else
		{
			emit_byte((u8)Op::NIL);
		}

		consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

		define_variable(global_index);
	}

	void declaration()
	{
		if (match(TokenType::FUN))
		{
			fun_declaration();
		}
		else if (match(TokenType::VAR))
		{
			var_declaration();
		}
		else
		{
			statement();
		}

		if (parser.panic_mode)
		{
			synchronize();
		}
	}

}	 // namespace CompilerImpl

Lox::ObjectFunction* Lox::compile(const char* source)
{
	using namespace CompilerImpl;

	init_scanner(source);

	Compiler compiler;
	init_compiler(&compiler, FunctionType::SCRIPT);

	parser.had_error = false;
	parser.panic_mode = false;

	advance();

	while (!match(TokenType::EOF_))
	{
		declaration();
	}

	ObjectFunction* function = end_compiler();
	return parser.had_error ? nullptr : function;
}

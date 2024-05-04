#include "compiler.h"
#include "chunk.h"
#include "scanner.h"

#include <array>
#include <cassert>
#include <format>
#include <iostream>
#include <string>

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
	Parser parser;
	Chunk* compiling_chunk;

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

	using ParseFn = void (*)();

	struct ParseRule
	{
		ParseFn prefix;
		ParseFn infix;
		Precedence precedence;
	};

	const ParseRule* get_rule(TokenType type);
	void expression();
	void parse_precedence(Precedence prec);

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
		return compiling_chunk;
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

	void emit_byte(u8 byte)
	{
		current_chunk()->write_chunk(byte, parser.previous.line);
	}

	void emit_bytes(u8 b1, u8 b2)
	{
		emit_byte(b1);
		emit_byte(b2);
	}

	void emit_return()
	{
		emit_byte((u8)Op::RETURN);
	}

	void emit_constant(Value value)
	{
		emit_bytes((u8)Op::CONSTANT, make_constant(value));
	}

	void end_compiler()
	{
		emit_return();

#if DEBUG_PRINT_CODE
		if (!parser.had_error)
		{
			current_chunk()->disassemble_chunk("code");
		}
#endif
	}

	void number()
	{
		double value = strtod(parser.previous.start, nullptr);
		emit_constant(value);
	}

	void expression()
	{
		parse_precedence(Precedence::ASSIGNMENT);
	}

	void unary()
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

	void binary()
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

	void grouping()
	{
		expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
	}

	void literal()
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
		prefix_rule();

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
			infix_rule();
		}
	}

	static constexpr std::array<ParseRule, (u8)TokenType::NUM> rules = []()
	{
		std::array<ParseRule, (u8)TokenType::NUM> result;

		// clang-format off
		//                                      prefix   infix    infix precedence
		result[(u8)TokenType::LEFT_PAREN]    = {grouping, nullptr,  Precedence::NONE};
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
		result[(u8)TokenType::IDENTIFIER]    = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::STRING]        = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::NUMBER]        = {number,   nullptr,  Precedence::NONE};
		result[(u8)TokenType::AND]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::CLASS]         = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::ELSE]          = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FALSE]         = {literal,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FOR]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::FUN]           = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::IF]            = {nullptr,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::NIL]           = {literal,  nullptr,  Precedence::NONE};
		result[(u8)TokenType::OR]            = {nullptr,  nullptr,  Precedence::NONE};
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

}	 // namespace CompilerImpl

bool Lox::compile(const char* source, Lox::Chunk& chunk)
{
	using namespace CompilerImpl;

	init_scanner(source);
	compiling_chunk = &chunk;
	parser.had_error = false;
	parser.panic_mode = false;

	advance();
	expression();
	consume(TokenType::EOF_, "Expected the end of an expression");
	end_compiler();
	return !parser.had_error;
}

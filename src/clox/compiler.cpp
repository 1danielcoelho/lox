#include "compiler.h"
#include "chunk.h"
#include "scanner.h"

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
	}

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

	// Will parse all expressions at 'prec' level or higher (higher value, so CALL > UNARY)
	void parse_precedence(Precedence prec)
	{
	}

	void number()
	{
		double value = strtod(parser.previous.start, NULL);
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

		// Compile right operand (+1 precedence value because these are left associative)
		ParserRule* rule = get_rule(op_type);
		parse_precedence((Precedence)(rule->precedence + 1));

		// clang-format off
		switch(op_type)
		{
			case TokenType::PLUS: 		emit_byte(Op::ADD); break;
			case TokenType::MINUS: 		emit_byte(Op::SUBTRACT); break;
			case TokenType::STAR: 		emit_byte(Op::MULTIPLY); break;
			case TokenType::SLASH: 		emit_byte(Op::DIVIDE); break;
			default:
			{
				assert(false);
				return;
			}
		}
		// clang-format on
	}

	void grouping()
	{
		expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
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

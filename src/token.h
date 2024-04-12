#pragma once

#include <stdint.h>
#include <string>
#include <variant>

namespace Lox
{
	enum class TokenType : uint8_t
	{
		// Single-character tokens.
		LEFT_PAREN,
		RIGHT_PAREN,
		LEFT_BRACE,
		RIGHT_BRACE,
		COMMA,
		DOT,
		MINUS,
		PLUS,
		SEMICOLON,
		SLASH,
		STAR,

		// One or two character tokens.
		BANG,
		BANG_EQUAL,
		EQUAL,
		EQUAL_EQUAL,
		GREATER,
		GREATER_EQUAL,
		LESS,
		LESS_EQUAL,

		// Literals.
		IDENTIFIER,
		STRING,
		NUMBER,

		// Keywords.
		AND,
		CLASS,
		ELSE,
		FALSE,
		FUN,
		FOR,
		IF,
		NIL,
		OR,
		PRINT,
		RETURN,
		SUPER,
		THIS,
		TRUE,
		VAR,
		WHILE,

		EOF_
	};

	class Token
	{
	public:
		TokenType type;
		std::string lexeme;
		std::variant<double, std::string> object;	 // TODO: This is not going to work, what if it's a class?
		uint32_t line;

	public:
		std::string to_string() const;
	};
}	 // namespace Lox

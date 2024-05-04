#pragma once

#include "common.h"

namespace Lox
{
	enum class TokenType : u8
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
		FOR,
		FUN,
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

		ERROR,
		EOF_,

		NUM
	};

	struct Token
	{
		TokenType type = TokenType::ERROR;
		const char* start = nullptr;
		i32 length = 0;
		i32 line = 1;

	public:
		Token() = default;
		Token(TokenType in_type);
        static Token error_token(const char* message);
	};

	void init_scanner(const char* source);
	Token scan_token();
}	 // namespace Lox

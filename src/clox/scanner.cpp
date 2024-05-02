#include "scanner.h"

#include <string>

namespace ScannerImpl
{
	struct Scanner
	{
		const char* start = nullptr;
		const char* current = nullptr;
		int line = 0;
	};

	Scanner scanner;

	bool is_at_end()
	{
		return *scanner.current == '\0';
	}

	char advance()
	{
		return *(scanner.current++);
	}

	// Consumes the character if it matches expected, and returns true if it did so
	bool match(char expected)
	{
		if (is_at_end())
		{
			return false;
		}

		if (*scanner.current != expected)
		{
			return false;
		}

		scanner.current++;
		return true;
	}

	char peek()
	{
		return *scanner.current;
	}

	char peek_next()
	{
		if (is_at_end())
		{
			return '\0';
		}

		return scanner.current[1];
	}

	void skip_whitespace()
	{
		while (true)
		{
			char c = peek();
			switch (c)
			{
				case ' ':
				case '\r':
				case '\t':
				{
					advance();
					break;
				}
				case '\n':
				{
					scanner.line++;
					advance();
					break;
				}
				case '/':
				{
					if (peek_next() == '/')
					{
						// Skip comment line
						while (peek() != '\n' && !is_at_end())
						{
							advance();
						}
					}
					else
					{
						return;
					}
					break;
				}
				default:
				{
					return;
				}
			}
		}
	}

	Lox::Token string()
	{
		while (peek() != '"' && !is_at_end())
		{
			if (peek() == '\n')
			{
				scanner.line++;
			}
			advance();
		}

		if (is_at_end())
		{
			return Lox::Token::error_token("Unterminated string");
		}

		// Consume closing quote
		advance();

		return Lox::Token{Lox::TokenType::STRING};
	}

	bool is_digit(char c)
	{
		return c >= '0' && c <= '9';
	}

	Lox::Token number()
	{
		// Consume all the digits
		while (is_digit(peek()))
		{
			advance();
		}

		if (peek() == '.' && is_digit(peek_next()))
		{
			// Consume the '.'
			advance();

			// Consume any digit after the '.'
			while (is_digit(peek()))
			{
				advance();
			}
		}

		return Lox::Token{Lox::TokenType::NUMBER};
	}

	bool is_alpha(char c)
	{
		return (c >= 'a' && c <= 'z') ||	//
			   (c >= 'A' && c <= 'Z') ||	//
			   c == '_';
	}

	Lox::TokenType check_keyword(i32 start, i32 length, const char* rest, Lox::TokenType type)
	{
		if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0)
		{
			return type;
		}

		return Lox::TokenType::IDENTIFIER;
	}

	Lox::TokenType identifier_type()
	{
		switch (scanner.start[0])
		{
			case 'a':
				return check_keyword(1, 2, "nd", Lox::TokenType::AND);
			case 'c':
				return check_keyword(1, 4, "lass", Lox::TokenType::CLASS);
			case 'e':
				return check_keyword(1, 3, "lse", Lox::TokenType::ELSE);
			case 'f':
			{
				if (scanner.current - scanner.start > 1)
				{
					switch (scanner.start[1])
					{
						case 'a':
							return check_keyword(2, 3, "lse", Lox::TokenType::FALSE);
						case 'o':
							return check_keyword(2, 1, "r", Lox::TokenType::FOR);
						case 'u':
							return check_keyword(2, 1, "n", Lox::TokenType::FUN);
					}
				}
				break;
			}
			case 'i':
				return check_keyword(1, 1, "f", Lox::TokenType::IF);
			case 'n':
				return check_keyword(1, 2, "il", Lox::TokenType::NIL);
			case 'o':
				return check_keyword(1, 1, "r", Lox::TokenType::OR);
			case 'p':
				return check_keyword(1, 4, "rint", Lox::TokenType::PRINT);
			case 'r':
				return check_keyword(1, 5, "eturn", Lox::TokenType::RETURN);
			case 's':
				return check_keyword(1, 4, "uper", Lox::TokenType::SUPER);
			case 't':
			{
				if (scanner.current - scanner.start > 1)
				{
					switch (scanner.start[1])
					{
						case 'h':
							return check_keyword(2, 2, "is", Lox::TokenType::THIS);
						case 'r':
							return check_keyword(2, 2, "ue", Lox::TokenType::TRUE);
					}
				}
				break;
			}
			case 'v':
				return check_keyword(1, 2, "ar", Lox::TokenType::VAR);
			case 'w':
				return check_keyword(1, 4, "hile", Lox::TokenType::WHILE);
		}

		return Lox::TokenType::IDENTIFIER;
	}

	Lox::Token identifier()
	{
		while (is_alpha(peek()) || is_digit(peek()))
		{
			advance();
		}

		return Lox::Token{identifier_type()};
	}
}	 // namespace ScannerImpl

Lox::Token::Token(TokenType in_type)
	: type(in_type)
	, start(ScannerImpl::scanner.start)
	, length((i32)(ScannerImpl::scanner.current - ScannerImpl::scanner.start))
	, line(ScannerImpl::scanner.line)
{
}

Lox::Token Lox::Token::error_token(const char* message)
{
	Token result{TokenType::ERROR};
	result.start = message;
	result.length = (i32)strlen(message);
	return result;
}

void Lox::init_scanner(const char* source)
{
	using namespace ScannerImpl;

	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

Lox::Token Lox::scan_token()
{
	using namespace ScannerImpl;

	skip_whitespace();

	scanner.start = scanner.current;

	if (is_at_end())
	{
		return Token{TokenType::EOF_};
	}

	char c = advance();

	if (is_alpha(c))
	{
		return identifier();
	}

	if (is_digit(c))
	{
		return number();
	}

	// clang-format off
	switch (c)
	{
        case '(': { return Token{TokenType::LEFT_PAREN}; break; }
        case ')': { return Token{TokenType::RIGHT_PAREN}; break; }
        case '{': { return Token{TokenType::LEFT_BRACE}; break; }
        case '}': { return Token{TokenType::RIGHT_BRACE}; break; }
        case ';': { return Token{TokenType::SEMICOLON}; break; }
        case ',': { return Token{TokenType::COMMA}; break; }
        case '.': { return Token{TokenType::DOT}; break; }
        case '-': { return Token{TokenType::MINUS}; break; }
        case '+': { return Token{TokenType::PLUS}; break; }
        case '/': { return Token{TokenType::SLASH}; break; }
        case '*': { return Token{TokenType::STAR}; break; }

        case '!': { return Token{match('=') ? TokenType::BANG_EQUAL    : TokenType::BANG};    break; }
        case '=': { return Token{match('=') ? TokenType::EQUAL_EQUAL   : TokenType::EQUAL};   break; }
        case '<': { return Token{match('=') ? TokenType::LESS_EQUAL    : TokenType::LESS};    break; }
        case '>': { return Token{match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER}; break; }

        case '"': { return string(); break; }
	}
	// clang-format on

	return Token::error_token("Unexpected character");
}

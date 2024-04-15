#include "tokenizer.h"
#include "error.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace TokenizerInternal
{
	bool is_digit(char c)
	{
		return c >= '0' && c <= '9';
	}

	bool is_alpha(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
	}

	bool is_alpha_numeric(char c)
	{
		return is_alpha(c) || is_digit(c);
	}

	// clang-format off
	std::unordered_map<std::string, Lox::TokenType> reserved_keywords{
        {"and",    Lox::TokenType::AND},
        {"class",  Lox::TokenType::CLASS},
        {"else",   Lox::TokenType::ELSE},
        {"false",  Lox::TokenType::FALSE},
        {"for",    Lox::TokenType::FOR},
        {"fun",    Lox::TokenType::FUN},
        {"if",     Lox::TokenType::IF},
        {"nil",    Lox::TokenType::NIL},
        {"or",     Lox::TokenType::OR},
        {"print",  Lox::TokenType::PRINT},
        {"return", Lox::TokenType::RETURN},
        {"super",  Lox::TokenType::SUPER},
        {"this",   Lox::TokenType::THIS},
        {"true",   Lox::TokenType::TRUE},
        {"var",    Lox::TokenType::VAR},
        {"while",  Lox::TokenType::WHILE},
	};
	// clang-format on
}

std::vector<Lox::Token> Lox::tokenize(const std::string& source)
{
	using namespace TokenizerInternal;

	std::vector<Token> result;

	uint32_t start = 0;
	uint32_t current = 0;
	uint32_t line = 1;

	auto advance = [&]() -> char
	{
		return source[current++];
	};

	auto is_at_end = [&]() -> bool
	{
		return current >= source.length();
	};

	auto peek = [&]() -> char
	{
		if (is_at_end())
		{
			return '\0';
		}

		return source[current];
	};

	auto peek_next = [&]() -> char
	{
		if (current + 1 >= source.length())
		{
			return '\0';
		}

		return source[current + 1];
	};

	auto add_token = [&](TokenType type, const Object& literal = {})
	{
		std::string substr = source.substr(start, current - start);
		std::cout << "consumed token type " << (int)type << " '" << substr.c_str() << "' with literal '" << to_string(literal) << "'" << std::endl;
		result.push_back(Token{type, substr, literal, line});
	};

	auto test_consume = [&](char expected)
	{
		if (is_at_end())
		{
			return false;
		}

		if (source[current] != expected)
		{
			return false;
		}

		current++;
		return true;
	};

	auto consume_string = [&]()
	{
		while (peek() != '"' && !is_at_end())
		{
			if (peek() == '\n')
			{
				++line;
			}

			advance();
		}

		if (is_at_end())
		{
			Lox::report_error(line, "Unterminated string.");
			return;
		}

		// Consume the closing "
		advance();

		std::string literal = source.substr(start + 1, current - start - 2);
		add_token(TokenType::STRING, literal);
	};

	auto consume_number = [&]()
	{
		while (is_digit(peek()))
		{
			advance();
		}

		if (peek() == '.' && is_digit(peek_next()))
		{
			// Consume the .
			advance();

			while (is_digit(peek()))
			{
				advance();
			}
		}

		add_token(TokenType::NUMBER, std::stof(source.substr(start, current - start)));
	};

	auto consume_identifier = [&]()
	{
		while (is_alpha_numeric(peek()))
		{
			advance();
		}

		std::string text = source.substr(start, current - start);

		TokenType type = TokenType::IDENTIFIER;
		auto iter = reserved_keywords.find(text);
		if (iter != reserved_keywords.end())
		{
			type = iter->second;
		}

		add_token(type);
	};

	while (!is_at_end())
	{
		start = current;
		char ch = advance();

		// clang-format off
		switch (ch)
		{
            case '(': add_token(TokenType::LEFT_PAREN); break;
            case ')': add_token(TokenType::RIGHT_PAREN); break;
            case '{': add_token(TokenType::LEFT_BRACE); break;
            case '}': add_token(TokenType::RIGHT_BRACE); break;
            case ',': add_token(TokenType::COMMA); break;
            case '.': add_token(TokenType::DOT); break;
            case '-': add_token(TokenType::MINUS); break;
            case '+': add_token(TokenType::PLUS); break;
            case ';': add_token(TokenType::SEMICOLON); break;
            case '*': add_token(TokenType::STAR); break;

            case '!': add_token(test_consume('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
            case '=': add_token(test_consume('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
            case '<': add_token(test_consume('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
            case '>': add_token(test_consume('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

            case '/':
            {
                // Comment goes until the end of the line
                if (test_consume('/'))
                {
                    while (peek() != '\n' && !is_at_end())
                    {
                        advance();
                    }
                }
                // Division
                else
                {
                    add_token(TokenType::SLASH);
                }

                break;
            }

            // Whitespace
            case ' ':
            case '\r':
            case '\t':
            {
                break;
            }

            case '\n':
            {
                line++;
                break;
            }

            case '"': consume_string(); break;

            default:
            {
                if (is_digit(ch))
                {
                    consume_number();
                }
                else if (is_alpha(ch))
                {
                    consume_identifier();
                }
                else
                {
                    Lox::report_error(line, "Unexpected character '" + std::string{ch} + "' (" + std::to_string((int)ch) + ")");
                }

                break;
            }
		};
		// clang-format on
	}

	assert(current == source.size());

	result.push_back(Token{TokenType::EOF_, "", {}, line});

	return result;
}

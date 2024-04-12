#include "tokenizer.h"
#include "error.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace TokenizerInternal
{
	bool is_digit(char c)
	{
		return c >= '0' && c <= '9';
	}
}

std::vector<Lox::Token> Lox::tokenize(const std::string& text)
{
	using namespace TokenizerInternal;

	std::vector<Token> result;

	uint32_t start = 0;
	uint32_t current = 0;
	uint32_t line = 1;

	auto advance = [&text, &current]() -> char
	{
		return text[current++];
	};

	auto is_at_end = [&current, &text]() -> bool
	{
		return current >= text.length();
	};

	auto peek = [&is_at_end, &text, &current]() -> char
	{
		if (is_at_end())
		{
			return '\0';
		}

		return text[current];
	};

	auto add_token = [&start, &current, &line, &result, &text](TokenType type, const LiteralVariantType& literal = {})
	{
		std::string substr = text.substr(start, current - start);
		std::cout << "consumed token '" << substr.c_str() << "' with literal '" << to_string(literal) << "'" << std::endl;
		result.push_back(Token{type, substr, literal, line});
	};

	auto test_consume = [&is_at_end, &current, &text](char expected)
	{
		if (is_at_end())
		{
			return false;
		}

		if (text[current] != expected)
		{
			return false;
		}

		current++;
		return true;
	};

	auto consume_string = [&text, &current, &line, &start, &add_token, &peek, &is_at_end, &advance]()
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

		std::string literal = text.substr(start + 1, current - start - 2);
		add_token(TokenType::STRING, literal);
	};

	// auto consume_number = [&stream, &current, &line, &text, &start, &add_token]()
	// {
	// 	while (is_digit(stream.peek()))
	// 	{
	// 		char ch;
	// 		stream.get(ch);
	// 		++current;
	// 	}

	// 	if (stream.peek() == '.' && is_digit(peek_nex()))
	// 	{
	// 		// Consume the .
	// 		char ch;
	// 		stream.get(ch);
	// 		++current;

	// 		while (is_digit(stream.peek()))
	// 		{
	// 			char ch;
	// 			stream.get(ch);
	// 			++current;
	// 		}
	// 	}

	// 	add_token(TokenType::NUMBER, std::stof(text.substr(start, current - start)));
	// };

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
                    // consume_number();
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

	assert(current == text.size());

	result.push_back(Token{TokenType::EOF_, "", {}, line});

	return result;
}

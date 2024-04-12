#include "tokenizer.h"
#include "error.h"

#include <cassert>
#include <iostream>
#include <sstream>

std::vector<Lox::Token> Lox::tokenize(const std::string& text)
{
	std::vector<Token> result;

	uint32_t start = 0;
	uint32_t current = 0;
	uint32_t line = 1;

	std::stringstream stream{text};

	auto add_token = [&start, &current, &line, &result, &text](TokenType type, const LiteralVariantType& literal = {})
	{
		std::string substr = text.substr(start, current - start);
		std::cout << "consumed token '" << substr.c_str() << "' with literal '" << to_string(literal) << "'" << std::endl;
		result.push_back(Token{type, substr, literal, line});
	};

	auto test_consume = [&stream, &current](char expected)
	{
		if (stream.eof())
		{
			return false;
		}

		if (stream.peek() == expected)
		{
			// Fully "consume" the character
			char ch;
			stream.get(ch);
			++current;
			return true;
		}

		return false;
	};

	auto consume_string = [&stream, &current, &line, &text, &start, &add_token]()
	{
		while (stream.peek() != '"' && !stream.eof())
		{
			if (stream.peek() == '\n')
			{
				++line;
			}

			char ch;
			stream.get(ch);
			++current;
		}

		if (stream.eof())
		{
			Lox::report_error(line, "Unterminated string.");
			return;
		}

		// Consume the closing "
		char ch;
		stream.get(ch);
		++current;

		std::string literal = text.substr(start + 1, current - start - 2);
		add_token(TokenType::STRING, literal);
	};

	char ch;
	while (stream.get(ch))
	{
		start = current;
		++current;

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
                    while (stream.peek() != '\n' && !stream.eof())
                    {
                        stream.get(ch);
                        ++current;
                    }

                    // TODO: reset start?
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

            default: Lox::report_error(line, "Unexpected character '" + std::string{ch} + "' (" + std::to_string((int)ch) + ")");
		};
		// clang-format on
	}

	assert(current == text.size());

	result.push_back(Token{TokenType::EOF_, "", {}, line});

	return result;
}

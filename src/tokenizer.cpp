#include "tokenizer.h"

#include <iostream>
#include <sstream>

std::vector<Token> tokenize(const std::string& text)
{
	std::vector<Token> result;

	uint32_t start = 0;
	uint32_t current = 0;
	uint32_t line = 1;

	auto add_token = [&start, &current, &line, &result, &text](TokenType type)
	{
		result.push_back(Token{type, text.substr(start, current - start), {}, line});
	};

	std::stringstream stream{text};
	while (!stream.eof())
	{
		char ch = stream.get();

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

            default: error(line, "Unexpected character '" + std::string{ch} + "'");
		};
		// clang-format on

		++current;

		std::cout << "c: '" << ch << "'" << std::endl;
	}

	result.push_back(Token{TokenType::EOF_, "", {}, line});

	return result;
}

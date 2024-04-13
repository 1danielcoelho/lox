#include "error.h"
#include "token.h"

#include <iostream>

namespace ErrorInternal
{
	bool had_error = false;
};

void Lox::report_error(int line, const std::string& message)
{
	std::cerr << "error: [line " << line << "] " << message << std::endl;
	ErrorInternal::had_error = true;
}

void Lox::report_error(const Lox::Token& token, const std::string& message)
{
	if (token.type == TokenType::EOF_)
	{
		std::cerr << "error: [line " << token.line << ", at end of file] " << message << std::endl;
	}
	else
	{
		std::cerr << "error: [line " << token.line << ", at '" << token.lexeme << "'] " << message << std::endl;
	}
	ErrorInternal::had_error = true;
}

bool Lox::had_error()
{
	return ErrorInternal::had_error;
}

void Lox::clear_error()
{
	ErrorInternal::had_error = false;
}

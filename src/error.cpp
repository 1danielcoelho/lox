#include "error.h"
#include "token.h"

#include <iostream>

namespace ErrorInternal
{
	bool had_error = false;
	bool had_runtime_error = false;
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

void Lox::report_runtime_error(const Lox::Token& token, const std::string& message)
{
	if (token.type == TokenType::EOF_)
	{
		std::cerr << "runtime error: [line " << token.line << ", at end of file] " << message << std::endl;
	}
	else
	{
		std::cerr << "runtime error: [line " << token.line << ", at '" << token.lexeme << "'] " << message << std::endl;
	}
	ErrorInternal::had_runtime_error = true;
}

bool Lox::had_error()
{
	return ErrorInternal::had_error;
}

void Lox::clear_error()
{
	ErrorInternal::had_error = false;
}

bool Lox::had_runtime_error()
{
	return ErrorInternal::had_runtime_error;
}

void Lox::clear_runtime_error()
{
	ErrorInternal::had_runtime_error = false;
}

#include "error.h"

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

bool Lox::had_error()
{
	return ErrorInternal::had_error;
}

void Lox::clear_error()
{
	ErrorInternal::had_error = false;
}

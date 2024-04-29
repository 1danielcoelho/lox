#pragma once

#include "object.h"

#include <string>

namespace Lox
{
	class Token;

	class ParseError
	{
	};

	struct RuntimeError
	{
		const Lox::Token& token;
		std::string error_message;
	};

	// We throw this in order to evaluate return statements
	struct Return
	{
		Lox::Object value;
	};

	// https://man.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html
	enum ErrorCodes
	{
		SUCCESS = 0,
		USAGE = 64,
		DATAERR = 65,
		NOINPUT = 66,
		SOFTWARE = 70,
		IOERR = 74,
	};

	void report_error(int line, const std::string& message);
	void report_error(const Token& token, const std::string& message);
	void report_runtime_error(const Token& token, const std::string& message);

	bool had_error();
	void clear_error();

	bool had_runtime_error();
	void clear_runtime_error();
}	 // namespace Lox

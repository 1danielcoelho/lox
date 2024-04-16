#pragma once

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

	// https://man.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html
	enum ErrorCodes
	{
		ERROR_CODE_SUCCESS = 0,
		ERROR_CODE_USAGE = 64,
		ERROR_CODE_DATAERR = 65,
		ERROR_CODE_NOINPUT = 66,
		ERROR_CODE_SOFTWARE = 70,
		ERROR_CODE_IOERR = 74,
	};

	void report_error(int line, const std::string& message);
	void report_error(const Token& token, const std::string& message);
	void report_runtime_error(const Token& token, const std::string& message);

	bool had_error();
	void clear_error();

	bool had_runtime_error();
	void clear_runtime_error();
}	 // namespace Lox

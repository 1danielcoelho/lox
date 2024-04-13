#pragma once

#include <string>

namespace Lox
{
	class Token;

	enum ErrorCodes
	{
		ERROR_CODE_SUCCESS = 0,
		ERROR_CODE_USAGE = 64,
		ERROR_CODE_DATAERR = 65,
		ERROR_CODE_NOINPUT = 66,
		ERROR_CODE_IOERR = 74,
	};

	void report_error(int line, const std::string& message);
	void report_error(const Token& token, const std::string& message);
	bool had_error();
	void clear_error();
}

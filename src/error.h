#pragma once

#include <string>

namespace Lox
{
	enum ErrorCodes
	{
		ERROR_CODE_SUCCESS = 0,
		ERROR_CODE_USAGE = 64,
		ERROR_CODE_DATAERR = 65,
		ERROR_CODE_NOINPUT = 66,
		ERROR_CODE_IOERR = 74,
	};

	void report_error(int line, const std::string& message);
	bool had_error();
	void clear_error();
}

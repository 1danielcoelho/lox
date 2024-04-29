#pragma once

#include "chunk.h"

#include <string>

namespace Lox
{
	enum class InterpretResult
	{
		OK,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	void initVM();
	InterpretResult interpret(const char* source);
	void freeVM();
}

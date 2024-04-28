#pragma once

#include "chunk.h"

namespace Lox
{
	enum class InterpretResult
	{
		OK,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	void initVM();
    InterpretResult interpret(const Chunk& chunk);
	void freeVM();
}

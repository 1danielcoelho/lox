#pragma once

namespace Lox
{
	class Chunk;
	class ObjectFunction;

	ObjectFunction* compile(const char* source);
	void mark_compiler_roots();
}

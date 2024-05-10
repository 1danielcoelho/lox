#pragma once

namespace Lox
{
	class Chunk;
	class ObjectFunction;

	ObjectFunction* compile(const char* source);
}

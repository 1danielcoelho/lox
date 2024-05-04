#pragma once

namespace Lox
{
	class Chunk;

	bool compile(const char* source, Chunk& chunk);
}

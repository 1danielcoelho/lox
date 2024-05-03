#pragma once

namespace Lox
{
	struct Chunk;

	bool compile(const char* source, Chunk& chunk);
}

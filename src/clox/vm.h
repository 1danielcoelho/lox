#pragma once

#include "chunk.h"

#include <string>
#include <unordered_map>

namespace Lox
{
	enum class InterpretResult
	{
		OK,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	class VM
	{
	public:
		const Lox::Chunk* chunk = nullptr;
		const u8* ip = nullptr;	   // Points to the *next* instruction to execute

		// TODO: Maybe use a std::array and fixed size? This looks simple though. It would be neat to see the
		// performance benefit of having the stack on the actual, uh, stack, later
		std::vector<Lox::Value> stack;

		Lox::Object* objects = nullptr;

		// Where we collect interned strings
		std::unordered_map<std::string, Lox::ObjectString*> strings;

		// Global variables stored by hash of the name string
		std::unordered_map<std::string, Lox::Value> globals;
	};

	extern VM vm;

	void init_VM();
	InterpretResult interpret(const char* source);
	void free_VM();
}

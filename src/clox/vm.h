#pragma once

#include "chunk.h"

#include <array>
#include <string>
#include <unordered_map>

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * (UINT8_MAX + 1))

namespace Lox
{
	class ObjectUpvalue;

	enum class InterpretResult
	{
		OK,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	struct CallFrame
	{
		ObjectClosure* closure = nullptr;
		u8* ip = nullptr;		   // Where to jump back to after the call is complete, in the caller's bytecode (maybe?)
		Value* slots = nullptr;	   // Points to the VM's value stack at the first slot this function can use
	};

	class VM
	{
	public:
		std::array<CallFrame, FRAMES_MAX> frames;
		i32 frames_position = 0;	// Points at the *next free position*

		std::array<Lox::Value, STACK_MAX> stack;
		i32 stack_position = 0;	   // Points at the *next free position*

		Lox::Object* objects = nullptr;

		// Where we collect interned strings
		std::unordered_map<std::string, Lox::ObjectString*> strings;

		ObjectUpvalue* open_upvalues = nullptr;

		// Global variables stored by hash of the name string
		std::unordered_map<std::string, Lox::Value> globals;
	};

	extern VM vm;

	void init_VM();
	InterpretResult interpret(const char* source);
	void free_VM();
}	 // namespace Lox

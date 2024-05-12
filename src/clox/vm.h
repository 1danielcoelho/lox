#pragma once

#include "chunk.h"
#include "common.h"

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
		// The book basically has a hash set here. By using the underlying string as key
		// we can kind of get the same behavior without having to implement std::hash for the pointer type itself.
		// We'll have an extra copy of the Lox::String I guess, but I don't particularly care about that yet
		//
		// TODO: Using Lox::Map here is troublesome because we don't want to run garbage collection and
		// try traversing the map *while it is allocating internal nodes*, as that seems to crash
		std::unordered_map<Lox::String, Lox::ObjectString*> strings;

		ObjectUpvalue* open_upvalues = nullptr;

		// Global variables stored by hash of the name string
		std::unordered_map<Lox::ObjectString*, Lox::Value> globals;

		// vector and not Lox::Vec as the garbage collector shouldn't manage this
		std::vector<Lox::Object*> gray_stack;
	};

	extern VM vm;

	void init_VM();
	void push(Lox::Value value);
	Lox::Value pop();
	InterpretResult interpret(const char* source);
	void free_VM();
}	 // namespace Lox

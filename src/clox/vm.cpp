#include "vm.h"

#include <cassert>
#include <iostream>

namespace VMImpl
{
	// TODO: Not quite sure why I need a class if I could just store internal state here...
	// on the book the class is declared in the header, and it seems that external code will
	// reference VM internals though?
	class VM
	{
	public:
		const Lox::Chunk* chunk = nullptr;
		const u8* ip = nullptr;	   // Points to the *next* instruction to execute
	};

	VM vm;

	u8 read_byte()
	{
		return *vm.ip++;
	}

	Lox::Value read_constant()
	{
		return vm.chunk->constants[read_byte()];
	}

	Lox::InterpretResult run()
	{
		while (true)
		{
			Lox::Op instruction = static_cast<Lox::Op>(read_byte());
			switch (instruction)
			{
				case Lox::Op::RETURN:
				{
					return Lox::InterpretResult::OK;
					break;
				}
				case Lox::Op::CONSTANT:
				{
					Lox::Value constant = read_constant();
					std::cout << constant << std::endl;
					break;
				}
				default:
				{
					assert(false);
				}
			}
		}
	}
}	 // namespace VMImpl

void Lox::initVM()
{
}

Lox::InterpretResult Lox::interpret(const Lox::Chunk& chunk)
{
	using namespace VMImpl;

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code.data();
	return run();
}

void Lox::freeVM()
{
}

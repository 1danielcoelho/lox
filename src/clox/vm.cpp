#include "vm.h"
#include "compiler.h"

#include <cassert>
#include <iostream>

#define DEBUG_TRACE_EXECUTION 1

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

		// TODO: Maybe use a std::array and fixed size? This looks simple though. It would be neat to see the
		// performance benefit of having the stack on the actual, uh, stack, later
		std::vector<Lox::Value> stack;
	};

	VM vm;

	void push(Lox::Value value)
	{
		vm.stack.push_back(value);
	}

	Lox::Value pop()
	{
		Lox::Value last = vm.stack.back();
		vm.stack.pop_back();
		return last;
	}

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
#if DEBUG_TRACE_EXECUTION
			for (const Lox::Value& value : vm.stack)
			{
				std::cout << "[ " << value << " ]";
			}
			std::cout << std::endl;

			vm.chunk->disassemble_instruction((i32)(vm.ip - vm.chunk->code.data()));
#endif

			Lox::Op instruction = static_cast<Lox::Op>(read_byte());
			switch (instruction)
			{
				case Lox::Op::RETURN:
				{
					std::cout << pop() << std::endl;
					return Lox::InterpretResult::OK;
					break;
				}
				case Lox::Op::ADD:
				{
					Lox::Value b = pop();
					Lox::Value a = pop();
					push(a + b);
					break;
				}
				case Lox::Op::SUBTRACT:
				{
					Lox::Value b = pop();
					Lox::Value a = pop();
					push(a - b);
					break;
				}
				case Lox::Op::MULTIPLY:
				{
					Lox::Value b = pop();
					Lox::Value a = pop();
					push(a * b);
					break;
				}
				case Lox::Op::DIVIDE:
				{
					Lox::Value b = pop();
					Lox::Value a = pop();
					push(a / b);
					break;
				}
				case Lox::Op::NEGATE:
				{
					push(-pop());
					break;
				}
				case Lox::Op::CONSTANT:
				{
					Lox::Value constant = read_constant();
					push(constant);
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

Lox::InterpretResult Lox::interpret(const char* source)
{
	using namespace VMImpl;

	Chunk chunk;
	if (!Lox::compile(source, chunk))
	{
		return Lox::InterpretResult::COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code.data();

	return run();
}

void Lox::freeVM()
{
}

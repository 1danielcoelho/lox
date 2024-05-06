#include "vm.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"

#include <cassert>
#include <format>
#include <iostream>
#include <string>

namespace Lox
{
	VM vm;
}

namespace VMImpl
{
	using namespace Lox;

	void runtime_error(const char* message)
	{
		std::cerr << message << std::endl;

		// The -1 is because the interpreter advances past each instruction before executing it,
		// so if we failed now the bad line was one before the current one
		size_t instruction = vm.ip - vm.chunk->code.data() - 1;
		i32 line = vm.chunk->lines[instruction];
		std::cerr << std::format("[line {}] in script", line) << std::endl;

		vm.stack.clear();
	}

	bool is_falsey(Value value)
	{
		if (std::holds_alternative<nullptr_t>(value))
		{
			return true;
		}

		if (std::holds_alternative<bool>(value))
		{
			return !std::get<bool>(value);
		}

		return false;
	}

	void push(Value value)
	{
		vm.stack.push_back(value);
	}

	Value pop()
	{
		Value last = vm.stack.back();
		vm.stack.pop_back();
		return last;
	}

	Value peek(i32 distance)
	{
		return vm.stack[vm.stack.size() - 1 - distance];
	}

	u8 read_byte()
	{
		return *vm.ip++;
	}

	Value read_constant()
	{
		return vm.chunk->constants[read_byte()];
	}

	void concatenate()
	{
		ObjectString* b = as_string(pop());
		ObjectString* a = as_string(pop());

		ObjectString* concat = Lox::ObjectString::allocate(a->get_string() + b->get_string());
		push(concat);
	}

	InterpretResult run()
	{
		while (true)
		{
#if DEBUG_TRACE_EXECUTION
			for (const Value& value : vm.stack)
			{
				std::cout << "[ " << to_string(value) << " ]";
			}
			std::cout << std::endl;

			vm.chunk->disassemble_instruction((i32)(vm.ip - vm.chunk->code.data()));
#endif

			Op instruction = static_cast<Op>(read_byte());
			switch (instruction)
			{
				case Op::CONSTANT:
				{
					Value constant = read_constant();
					push(constant);
					break;
				}
				case Op::NIL:
				{
					push(nullptr);
					break;
				}
				case Op::TRUE:
				{
					push(true);
					break;
				}
				case Op::FALSE:
				{
					push(false);
					break;
				}
				case Op::POP:
				{
					pop();
					break;
				}
				case Op::GET_GLOBAL:
				{
					// Variable name is stored as a constant
					Value constant = read_constant();
					Lox::ObjectString* obj_string = as_string(constant);
					const std::string& variable_name = obj_string->get_string();

					// Check to see if we have a value for that variable
					auto iter = vm.globals.find(variable_name);
					if (iter == vm.globals.end())
					{
						std::string error_message = std::format("Undefined variable '{}'", variable_name);
						runtime_error(error_message.c_str());
						return Lox::InterpretResult::RUNTIME_ERROR;
					}

					// Push that value into the stack
					push(iter->second);
					break;
				}
				case Op::DEFINE_GLOBAL:
				{
					// Variable name is stored as a constant
					Value constant = read_constant();
					Lox::ObjectString* obj_string = as_string(constant);

					vm.globals[obj_string->get_string()] = peek(0);	   // Initializer value
					pop();
					break;
				}
				case Op::EQUAL:
				{
					Value b = pop();
					Value a = pop();
					push(values_equal(a, b));
					break;
				}
				case Op::GREATER:
				{
					if (!is_number(peek(0)) || !is_number(peek(1)))
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}
					Value b = pop();
					Value a = pop();
					push(std::get<f64>(a) > std::get<f64>(b));
					break;
				}
				case Op::LESS:
				{
					if (!is_number(peek(0)) || !is_number(peek(1)))
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}
					Value b = pop();
					Value a = pop();
					push(std::get<f64>(a) < std::get<f64>(b));
					break;
				}
				case Op::ADD:
				{
					if (is_string(peek(0)) && is_string(peek(1)))
					{
						concatenate();
					}
					else if (is_number(peek(0)) && is_number(peek(1)))
					{
						Value b = pop();
						Value a = pop();
						push(std::get<f64>(a) + std::get<f64>(b));
					}
					else
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}

					break;
				}
				case Op::SUBTRACT:
				{
					if (!is_number(peek(0)) || !is_number(peek(1)))
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}
					Value b = pop();
					Value a = pop();
					push(std::get<f64>(a) - std::get<f64>(b));
					break;
				}
				case Op::MULTIPLY:
				{
					if (!is_number(peek(0)) || !is_number(peek(1)))
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}
					Value b = pop();
					Value a = pop();
					push(std::get<f64>(a) * std::get<f64>(b));
					break;
				}
				case Op::DIVIDE:
				{
					if (!is_number(peek(0)) || !is_number(peek(1)))
					{
						runtime_error("Operands must be numbers");
						return InterpretResult::RUNTIME_ERROR;
					}
					Value b = pop();
					Value a = pop();
					push(std::get<f64>(a) / std::get<f64>(b));
					break;
				}
				case Op::NOT:
				{
					push(is_falsey(pop()));
					break;
				}
				case Op::NEGATE:
				{
					if (!is_number(peek(0)))
					{
						runtime_error("Operand must be a number");
						return InterpretResult::RUNTIME_ERROR;
					}

					push(-std::get<f64>(pop()));
					break;
				}
				case Op::PRINT:
				{
					std::cout << to_string(pop()) << std::endl;
					break;
				}
				case Op::RETURN:
				{
					return InterpretResult::OK;
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

void Lox::init_VM()
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

void Lox::free_VM()
{
	free_objects();
	vm.strings.clear();
	vm.globals.clear();
}

#include "vm.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"

#include <cassert>
#include <format>
#include <iostream>
#include <string>

#include <time.h>

namespace Lox
{
	VM vm;
}

namespace VMImpl
{
	using namespace Lox;

	Value clock_native([[maybe_unused]] i32 arg_count, [[maybe_unused]] Value* args)
	{
		return (double)clock() / CLOCKS_PER_SEC;
	}

	void reset_stack()
	{
		vm.stack_position = 0;
		vm.frames_position = 0;
	}

	void runtime_error(const char* message)
	{
		std::cerr << message << std::endl;

		for (i32 i = vm.frames_position - 1; i >= 0; i--)
		{
			CallFrame* frame = &vm.frames[i];
			ObjectFunction* function = frame->closure->function;
			size_t instruction = frame->ip - function->chunk.code.data() - 1;	 // -1 because the ip points at th enext instruction, and we
																				 // want to report about the one that failed (last one)
			std::cerr << std::format("[line {}] in ", function->chunk.lines[instruction]);

			if (function->name == nullptr)
			{
				std::cerr << "script" << std::endl;
			}
			else
			{
				std::cerr << function->name->get_string() << "()" << std::endl;
			}
		}

		reset_stack();
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
		vm.stack[vm.stack_position++] = value;
	}

	Value pop()
	{
		return vm.stack[--vm.stack_position];
	}

	Value peek(i32 distance)
	{
		return vm.stack[vm.stack_position - 1 - distance];
	}

	u8 read_byte(CallFrame* frame)
	{
		return *frame->ip++;
	}

	u16 read_short(CallFrame* frame)
	{
		frame->ip += 2;
		return (u16)((frame->ip[-2] << 8) | frame->ip[-1]);
	}

	Value read_constant(CallFrame* frame)
	{
		return frame->closure->function->chunk.constants[read_byte(frame)];
	}

	void concatenate()
	{
		ObjectString* b = as_string(pop());
		ObjectString* a = as_string(pop());

		ObjectString* concat = Lox::ObjectString::allocate(a->get_string() + b->get_string());
		push(concat);
	}

	void define_native(const char* name, NativeFn function)
	{
		push(ObjectString::allocate(name));
		push(ObjectNativeFunction::allocate(function));

		// TODO: Why not relative to current stack pos?
		// TODO: This is kind of silly: Given how I'm using std::unordered_map<std::string,...> I could just construct
		// a new std::string here? It seems to be a GC thing
		vm.globals[as_string(vm.stack[0])->get_string()] = vm.stack[1];

		pop();
		pop();
	}

	bool call(ObjectClosure* closure, i32 arg_count)
	{
		if (arg_count != closure->function->arity)
		{
			runtime_error(std::format("Expected {} arguments but got {}", closure->function->arity, arg_count).c_str());
			return false;
		}

		if (vm.frames_position == FRAMES_MAX)
		{
			runtime_error("Stack overflow");
			return false;
		}

		CallFrame* frame = &vm.frames[vm.frames_position++];
		frame->closure = closure;
		frame->ip = closure->function->chunk.code.data();
		frame->slots = &vm.stack[vm.stack_position - arg_count - 1];	// The -1 accounts for stack slot zero, which the compiler sets aside
		return true;
	}

	bool call_value(Value callee, i32 arg_count)
	{
		if (Object* callee_object = as_object(callee))
		{
			if (ObjectClosure* closure = dynamic_cast<ObjectClosure*>(callee_object))
			{
				return call(closure, arg_count);
			}
			else if (ObjectNativeFunction* native = dynamic_cast<ObjectNativeFunction*>(callee_object))
			{
				NativeFn native_func = native->function;
				Value result = native_func(arg_count, &vm.stack[vm.stack_position] - arg_count);
				vm.stack_position -= arg_count + 1;
				push(result);
				return true;
			}
		}

		runtime_error("Can only call functions and classes");
		return false;
	}

	InterpretResult run()
	{
		CallFrame* frame = &vm.frames[vm.frames_position - 1];

#if DEBUG_TRACE_EXECUTION
		std::cout << "----------------------" << std::endl;
#endif

		while (true)
		{
#if DEBUG_TRACE_EXECUTION
			std::cout << "[";
			for (i32 index = 0; index < vm.stack_position; ++index)
			{
				const Value& value = vm.stack[index];
				std::cout << "[ " << to_string(value) << " ]";
			}
			std::cout << "]" << std::endl;

			frame->closure->function->chunk.disassemble_instruction((i32)(frame->ip - frame->closure->function->chunk.code.data()));
#endif

			Op instruction = static_cast<Op>(read_byte(frame));
			switch (instruction)
			{
				case Op::CONSTANT:
				{
					Value constant = read_constant(frame);
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
				case Op::GET_LOCAL:
				{
					// Yes this pushes a copy of the value back onto the stack. The idea being that
					// other bytecode instructions will look for data only at the top of the stack
					u8 slot = read_byte(frame);
					push(frame->slots[slot]);
					break;
				}
				case Op::SET_LOCAL:
				{
					// Note that it doesn't pop, as assignment is an expression and every expression produces
					// a value (here the assigned value itself). Tthe value is left at the top of the stack
					u8 slot = read_byte(frame);
					frame->slots[slot] = peek(0);
					break;
				}
				case Op::GET_GLOBAL:
				{
					// Variable name is stored as a constant
					Value constant = read_constant(frame);
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
					Value constant = read_constant(frame);
					Lox::ObjectString* obj_string = as_string(constant);

					vm.globals[obj_string->get_string()] = peek(0);	   // Initializer value
					pop();
					break;
				}
				case Op::SET_GLOBAL:
				{
					// Variable name is stored as a constant
					Value constant = read_constant(frame);
					Lox::ObjectString* obj_string = as_string(constant);
					const std::string& variable_name = obj_string->get_string();

					// Check to see if we have a variable declared for that name yet
					auto iter = vm.globals.find(variable_name);
					if (iter != vm.globals.end())
					{
						vm.globals[variable_name] = peek(0);
					}
					else
					{
						std::string error_message = std::format("Undefined variable '{}'", variable_name);
						runtime_error(error_message.c_str());
						return Lox::InterpretResult::RUNTIME_ERROR;
					}

					// Note: This doesn't pop the value off the stack, as assignment is an expression, so it
					// needs to leave that value there in case the assignment is nested inside some larger
					// expression
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
					std::cout << ">> " << to_string(pop()) << std::endl;
					break;
				}
				case Op::JUMP:
				{
					u16 offset = read_short(frame);
					frame->ip += offset;
					break;
				}
				case Op::JUMP_IF_FALSE:
				{
					u16 offset = read_short(frame);
					if (is_falsey(peek(0)))
					{
						frame->ip += offset;
					}
					break;
				}
				case Op::LOOP:
				{
					u16 offset = read_short(frame);
					frame->ip -= offset;
					break;
				}
				case Op::CALL:
				{
					u8 arg_count = read_byte(frame);
					if (!call_value(peek(arg_count), arg_count))
					{
						return InterpretResult::RUNTIME_ERROR;
					}
					// call_value created a new CallFrame
					frame = &vm.frames[vm.frames_position - 1];
					break;
				}
				case Op::CLOSURE:
				{
					ObjectFunction* function = as_function(read_constant(frame));
					ObjectClosure* closure = ObjectClosure::allocate(function);
					push(closure);
					break;
				}
				case Op::RETURN:
				{
					Value result = pop();
					vm.frames_position--;
					if (vm.frames_position == 0)
					{
						pop();
						return InterpretResult::OK;
					}

					vm.stack_position = (i32)(frame->slots - vm.stack.data());
					push(result);
					frame = &vm.frames[vm.frames_position - 1];
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
	using namespace VMImpl;

	reset_stack();

	define_native("clock", clock_native);
}

Lox::InterpretResult Lox::interpret(const char* source)
{
	using namespace VMImpl;

	// Compile the source code into a fake top-level "function"
	ObjectFunction* function = Lox::compile(source);
	if (function == nullptr)
	{
		return Lox::InterpretResult::COMPILE_ERROR;
	}

	// Force GC to retain the function while we allocate a new closure to wrap it with
	push(function);
	ObjectClosure* closure = ObjectClosure::allocate(function);
	pop();

	// Put the closure itself into stack slot zero (the compiler set this aside for us).
	push(closure);
	call(closure, 0);

	return run();
}

void Lox::free_VM()
{
	free_objects();
	vm.strings.clear();
	vm.globals.clear();
}

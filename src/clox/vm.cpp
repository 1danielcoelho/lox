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
		vm.open_upvalues = nullptr;
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
		// Peek them here because it keeps the values inside the stack while we allocate
		// a new ObjectString (which can trigger GC)
		ObjectString* b = as_string(peek(0));
		ObjectString* a = as_string(peek(1));

		ObjectString* concat = Lox::ObjectString::allocate(a->get_string() + b->get_string());
		pop();	  // Finally pop 'a' and 'b' off the stack
		pop();
		push(concat);
	}

	void define_native(const char* name, NativeFn function)
	{
		push(ObjectString::allocate(name));
		push(ObjectNativeFunction::allocate(function));

		// TODO: Why not relative to current stack pos?
		vm.globals[as_string(vm.stack[0])] = vm.stack[1];

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

	ObjectUpvalue* capture_upvalue(Value* local)
	{
		ObjectUpvalue* prev_upvalue = nullptr;
		ObjectUpvalue* upvalue = vm.open_upvalues;

		// Since locals and upvalues are in order, if we find an upvalue whose
		// local slot is below (i.e. earlier) than the one we're looking for, then
		// we've gone past the slot we're closing over, and there can't be an existing
		// upvalue for it
		while (upvalue != nullptr && upvalue->location > local)
		{
			prev_upvalue = upvalue;
			upvalue = upvalue->next_upvalue;
		}

		if (upvalue != nullptr && upvalue->location == local)
		{
			return upvalue;
		}

		ObjectUpvalue* created_upvalue = ObjectUpvalue::allocate(local);
		created_upvalue->next_upvalue = upvalue;

		if (prev_upvalue == nullptr)
		{
			vm.open_upvalues = created_upvalue;
		}
		else
		{
			prev_upvalue->next_upvalue = created_upvalue;
		}

		return created_upvalue;
	}

	void close_upvalues(Value* last)
	{
		while (vm.open_upvalues != nullptr && vm.open_upvalues->location >= last)
		{
			ObjectUpvalue* upvalue = vm.open_upvalues;
			upvalue->closed = *upvalue->location;
			upvalue->location = &upvalue->closed;
			vm.open_upvalues = upvalue->next_upvalue;
		}
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
					Lox::Value val = read_constant(frame);
					Lox::ObjectString* obj_string = as_string(val);
					const Lox::String& variable_name = obj_string->get_string();

					// Check to see if we have a value for that variable
					auto iter = vm.globals.find(obj_string);
					if (iter == vm.globals.end())
					{
						Lox::String error_message{std::format("Undefined variable '{}'", variable_name)};
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
					Lox::Value val = read_constant(frame);
					Lox::ObjectString* obj_string = as_string(val);

					vm.globals[obj_string] = peek(0);	 // Initializer value
					pop();
					break;
				}
				case Op::SET_GLOBAL:
				{
					// Variable name is stored as a constant
					Lox::Value val = read_constant(frame);
					Lox::ObjectString* obj_string = as_string(val);
					const Lox::String& variable_name = obj_string->get_string();

					// Check to see if we have a variable declared for that name yet
					auto iter = vm.globals.find(obj_string);
					if (iter != vm.globals.end())
					{
						vm.globals[obj_string] = peek(0);
					}
					else
					{
						Lox::String error_message{std::format("Undefined variable '{}'", variable_name)};
						runtime_error(error_message.c_str());
						return Lox::InterpretResult::RUNTIME_ERROR;
					}

					// Note: This doesn't pop the value off the stack, as assignment is an expression, so it
					// needs to leave that value there in case the assignment is nested inside some larger
					// expression
					break;
				}
				case Op::GET_UPVALUE:
				{
					u8 slot = read_byte(frame);
					push(*frame->closure->upvalues[slot]->location);
					break;
				}
				case Op::SET_UPVALUE:
				{
					u8 slot = read_byte(frame);
					*frame->closure->upvalues[slot]->location = peek(0);
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

					for (i32 i = 0; i < function->upvalue_count; ++i)
					{
						u8 is_local = read_byte(frame);
						u8 index = read_byte(frame);
						if (is_local)
						{
							closure->upvalues.push_back(capture_upvalue(frame->slots + index));
						}
						else
						{
							// When this executes, we're already on the "surrounding" function scope,
							// so if we need to fetch an upvalue we just look into our current frame
							closure->upvalues.push_back(frame->closure->upvalues[index]);
						}
					}

					break;
				}
				case Op::CLOSE_UPVALUE:
				{
					close_upvalues(&vm.stack[vm.stack_position] - 1);
					pop();
					break;
				}
				case Op::RETURN:
				{
					Value result = pop();
					close_upvalues(frame->slots);
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

void Lox::push(Lox::Value value)
{
	vm.stack[vm.stack_position++] = value;
}

Lox::Value Lox::pop()
{
	return vm.stack[--vm.stack_position];
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

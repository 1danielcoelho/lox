#include "memory.h"
#include "compiler.h"
#include "object.h"
#include "vm.h"

#include <iostream>

namespace MemoryImpl
{
	using namespace Lox;

	void free_object(Lox::Object* obj)
	{
		delete obj;
	}

	void mark_roots()
	{
		for (i32 stack_slot = 0; stack_slot < vm.stack_position; ++stack_slot)
		{
			Value* slot = &vm.stack[stack_slot];
			mark_value(*slot);
		}

		for (const auto& [str, val] : vm.globals)
		{
			mark_value(val);
		}

		for (i32 frame_index = 0; frame_index < vm.frames_position; ++frame_index)
		{
			mark_object(vm.frames[frame_index].closure);
		}

		for (ObjectUpvalue* upvalue = vm.open_upvalues; upvalue != nullptr; upvalue = upvalue->next_upvalue)
		{
			mark_object(upvalue);
		}

		mark_compiler_roots();
	}

	void blacken_object(Object* object)
	{
#if DEBUG_LOG_GC
		std::cout << std::format("{} blacken {}", (void*)object, Lox::to_string(object)) << std::endl;
#endif

		if (ObjectUpvalue* upvalue = dynamic_cast<ObjectUpvalue*>(object))
		{
			mark_value(upvalue->closed);
		}
		else if (ObjectFunction* function = dynamic_cast<ObjectFunction*>(object))
		{
			mark_object(function->name);
			for (const Value& val : function->chunk.constants)
			{
				mark_value(val);
			}
		}
		else if (ObjectClosure* closure = dynamic_cast<ObjectClosure*>(object))
		{
			mark_object(closure->function);
			for (ObjectUpvalue* upvalue : closure->upvalues)
			{
				mark_object(upvalue);
			}
		}
	}

	void trace_references()
	{
		while (vm.gray_stack.size() > 0)
		{
			Object* object = vm.gray_stack.back();
			vm.gray_stack.pop_back();

			blacken_object(object);
		}
	}

	void sweep()
	{
		Object* previous = nullptr;
		Object* object = vm.objects;
		while (object != nullptr)
		{
			if (object->is_marked)
			{
				object->is_marked = false;

				previous = object;
				object = object->next;
			}
			else
			{
				Object* unreached = object;

				// Reconnect previous to next
				object = object->next;
				if (previous != nullptr)
				{
					previous->next = object;
				}
				else
				{
					vm.objects = object;
				}

				unreached->free();
			}
		}
	}
}	 // namespace MemoryImpl

void Lox::mark_object(Object* object)
{
	if (object == nullptr || object->is_marked)
	{
		return;
	}

#if DEBUG_LOG_GC
	std::cout << std::format("{} mark {}", (void*)object, Lox::to_string(object)) << std::endl;
#endif

	object->is_marked = true;

	vm.gray_stack.push_back(object);
}

void Lox::mark_value(Value value)
{
	if (is_object(value))
	{
		Object* object = as_object(value);
		mark_object(object);
	}
}

void Lox::collect_garbage()
{
	using namespace MemoryImpl;

#if DEBUG_LOG_GC
	std::cout << "-- gc begin\n";
#endif

	mark_roots();
	trace_references();
	sweep();

#if DEBUG_LOG_GC
	std::cout << "-- gc end\n";
#endif
}

void Lox::free_objects()
{
	using namespace MemoryImpl;

	Object* object = vm.objects;
	while (object != nullptr)
	{
		Object* next = object->next;

		free_object(object);
		object = next;
	}

	vm.gray_stack.clear();
}

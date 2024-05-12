#include "memory.h"
#include "object.h"
#include "vm.h"

namespace MemoryImpl
{
	void free_object(Lox::Object* obj)
	{
		delete obj;
	}
}

void Lox::collect_garbage()
{
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
}

#include "object.h"
#include "memory.h"
#include "vm.h"

#include <cassert>
#include <format>

namespace ObjectImpl
{
	static Lox::TrackingAllocator<u8> allocator;

	template<typename T, typename... Args>
	T* allocate(Args&&... args)
	{
		u8* buf = allocator.allocate(sizeof(T));
		T* object = new (buf) T(std::forward<Args>(args)...);

		// TODO: Do we still need these?
		object->next = Lox::vm.objects;
		Lox::vm.objects = object;

		return object;
	}

	template<typename T>
	void free(void* ptr)
	{
		allocator.deallocate(static_cast<unsigned char*>(ptr), sizeof(T));
	}
}

Lox::String Lox::Object::to_string() const
{
	return "";
}

Lox::ObjectString* Lox::ObjectString::allocate(const Lox::String& string)
{
	auto iter = vm.strings.find(string);
	if (iter != vm.strings.end())
	{
		return iter->second;
	}

	Lox::ObjectString* instance = ObjectImpl::allocate<Lox::ObjectString>(string);

	vm.strings.insert({string, instance});

	return instance;
}

void Lox::ObjectString::free(ObjectString* instance)
{
	ObjectImpl::free<Lox::ObjectString>(instance);
}

Lox::ObjectString::ObjectString(const Lox::String& in_string)
	: string(in_string)
{
}

Lox::ObjectString::~ObjectString()
{
	vm.strings.erase(string);
}

Lox::String Lox::ObjectString::to_string() const
{
	return string;
}

const Lox::String& Lox::ObjectString::get_string() const
{
	return string;
}

Lox::ObjectFunction* Lox::ObjectFunction::allocate()
{
	Lox::ObjectFunction* instance = ObjectImpl::allocate<Lox::ObjectFunction>();
	instance->arity = 0;
	instance->upvalue_count = 0;
	instance->name = nullptr;
	return instance;
}

void Lox::ObjectFunction::free(ObjectFunction* instance)
{
	ObjectImpl::free<Lox::ObjectFunction>(instance);
}

Lox::String Lox::ObjectFunction::to_string() const
{
	return name ? Lox::String{std::format("<fn {}>", name->get_string())} : "<script>";
}

Lox::ObjectUpvalue* Lox::ObjectUpvalue::allocate(Value* slot)
{
	Lox::ObjectUpvalue* instance = ObjectImpl::allocate<Lox::ObjectUpvalue>(slot);
	instance->location = slot;
	instance->closed = nullptr;
	instance->next_upvalue = nullptr;
	return instance;
}

void Lox::ObjectUpvalue::free(ObjectUpvalue* instance)
{
	ObjectImpl::free<Lox::ObjectUpvalue>(instance);
}

Lox::ObjectUpvalue::ObjectUpvalue(Value* in_slot)
	: location(in_slot)
{
}

Lox::String Lox::ObjectUpvalue::to_string() const
{
	return "upvalue";
}

Lox::ObjectClosure* Lox::ObjectClosure::allocate(ObjectFunction* function)
{
	Lox::ObjectClosure* instance = ObjectImpl::allocate<Lox::ObjectClosure>(function);
	instance->function = function;
	instance->upvalues.reserve(function->upvalue_count);
	return instance;
}

void Lox::ObjectClosure::free(ObjectClosure* instance)
{
	ObjectImpl::free<Lox::ObjectClosure>(instance);
}

Lox::ObjectClosure::ObjectClosure(ObjectFunction* in_function)
	: function(in_function)
{
}

Lox::String Lox::ObjectClosure::to_string() const
{
	return function->to_string();
}

Lox::ObjectNativeFunction* Lox::ObjectNativeFunction::allocate(NativeFn in_function)
{
	Lox::ObjectNativeFunction* instance = ObjectImpl::allocate<Lox::ObjectNativeFunction>(in_function);
	return instance;
}

void Lox::ObjectNativeFunction::free(ObjectNativeFunction* instance)
{
	ObjectImpl::free<Lox::ObjectNativeFunction>(instance);
}

Lox::ObjectNativeFunction::ObjectNativeFunction(NativeFn in_function)
	: function(in_function)
{
}

Lox::String Lox::ObjectNativeFunction::to_string() const
{
	return "<native fn>";
}

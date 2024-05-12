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

	// Make sure instance is marked as a root if we trigger GC on the insert call below it.
	// Here we don't *need* to do this because vm.strings is not tracked by GC (although it should...)
	push(instance);
	vm.strings.insert({string, instance});
	pop();

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

void Lox::ObjectString::free()
{
	Lox::ObjectString::free(this);
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

void Lox::ObjectFunction::free()
{
	Lox::ObjectFunction::free(this);
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

void Lox::ObjectUpvalue::free()
{
	Lox::ObjectUpvalue::free(this);
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

void Lox::ObjectClosure::free()
{
	Lox::ObjectClosure::free(this);
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

void Lox::ObjectNativeFunction::free()
{
	Lox::ObjectNativeFunction::free(this);
}

Lox::String Lox::ObjectNativeFunction::to_string() const
{
	return "<native fn>";
}

Lox::ObjectClass* Lox::ObjectClass::allocate(Lox::ObjectString* in_name)
{
	Lox::ObjectClass* cls = ObjectImpl::allocate<Lox::ObjectClass>(in_name);
	return cls;
}

void Lox::ObjectClass::free(ObjectClass* instance)
{
	ObjectImpl::free<Lox::ObjectClass>(instance);
}

Lox::ObjectClass::ObjectClass(ObjectString* in_name)
	: name(in_name)
{
}

void Lox::ObjectClass::free()
{
	Lox::ObjectClass::free(this);
}

Lox::String Lox::ObjectClass::to_string() const
{
	return Lox::String{std::format("class {}", name->get_string())};
}

Lox::ObjectInstance* Lox::ObjectInstance::allocate(ObjectClass* klass)
{
	Lox::ObjectInstance* instance = ObjectImpl::allocate<Lox::ObjectInstance>(klass);
	return instance;
}

void Lox::ObjectInstance::free(ObjectInstance* instance)
{
	ObjectImpl::free<Lox::ObjectInstance>(instance);
}

Lox::ObjectInstance::ObjectInstance(Lox::ObjectClass* in_klass)
	: klass(in_klass)
{
}

void Lox::ObjectInstance::free()
{
	Lox::ObjectInstance::free(this);
}

Lox::String Lox::ObjectInstance::to_string() const
{
	return Lox::String{std::format("{} instance", klass->to_string())};
}

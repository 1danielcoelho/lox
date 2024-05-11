#include "object.h"
#include "vm.h"

#include <format>

std::string Lox::Object::to_string() const
{
	return "";
}

Lox::ObjectString* Lox::ObjectString::allocate(const std::string& string)
{
	auto iter = vm.strings.find(string);
	if (iter != vm.strings.end())
	{
		return iter->second;
	}

	// TODO: Move to base class?
	Lox::ObjectString* result = new Lox::ObjectString(string);
	result->next = vm.objects;
	vm.objects = result;

	vm.strings.insert({string, result});

	return result;
}

Lox::ObjectString::~ObjectString()
{
	vm.strings.erase(string);
}

std::string Lox::ObjectString::to_string() const
{
	return string;
}

const std::string& Lox::ObjectString::get_string() const
{
	return string;
}

Lox::ObjectString::ObjectString(const std::string& in_string)
	: string(in_string)
{
}

Lox::ObjectFunction* Lox::ObjectFunction::allocate()
{
	Lox::ObjectFunction* function = new Lox::ObjectFunction();
	function->next = vm.objects;
	vm.objects = function;

	function->arity = 0;
	function->upvalue_count = 0;
	function->name = nullptr;
	return function;
}

std::string Lox::ObjectFunction::to_string() const
{
	return name ? std::format("<fn {}>", name->get_string()) : "<script>";
}

Lox::ObjectClosure* Lox::ObjectClosure::allocate(ObjectFunction* function)
{
	Lox::ObjectClosure* closure = new Lox::ObjectClosure();
	closure->next = vm.objects;
	vm.objects = closure;

	closure->function = function;
	closure->upvalues.reserve(function->upvalue_count);
	return closure;
}

std::string Lox::ObjectClosure::to_string() const
{
	return function->to_string();
}

Lox::ObjectUpvalue* Lox::ObjectUpvalue::allocate(Value* slot)
{
	Lox::ObjectUpvalue* upvalue = new Lox::ObjectUpvalue();
	upvalue->next = vm.objects;
	vm.objects = upvalue;

	upvalue->location = slot;
	return upvalue;
}

std::string Lox::ObjectUpvalue::to_string() const
{
	return "upvalue";
}

Lox::ObjectNativeFunction* Lox::ObjectNativeFunction::allocate(NativeFn in_function)
{
	Lox::ObjectNativeFunction* function_obj = new Lox::ObjectNativeFunction();
	function_obj->next = vm.objects;
	vm.objects = function_obj;

	function_obj->function = in_function;
	return function_obj;
}

std::string Lox::ObjectNativeFunction::to_string() const
{
	return "<native fn>";
}

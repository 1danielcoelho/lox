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
	function->name = nullptr;
	return function;
}

std::string Lox::ObjectFunction::to_string() const
{
	return name ? std::format("<fn {}>", name->get_string()) : "<script>";
}

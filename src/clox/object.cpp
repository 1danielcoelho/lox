#include "object.h"
#include "vm.h"

std::string Lox::Object::to_string() const
{
	return "";
}

Lox::ObjectString* Lox::ObjectString::allocate(const std::string& string)
{
	// TODO: This is not great because now the language really just can't store
	// different strings that happen to hash collide... It's probably fine for now though
	size_t hash = std::hash<std::string>()(string);
	auto iter = vm.strings.find(hash);
	if (iter != vm.strings.end())
	{
		return iter->second;
	}

	// TODO: Move to base class?
	Lox::ObjectString* result = new Lox::ObjectString(string);
	result->next = vm.objects;
	vm.objects = result;

	vm.strings.insert({hash, result});

	return result;
}

Lox::ObjectString::~ObjectString()
{
	size_t hash = std::hash<std::string>()(string);
	vm.strings.erase(hash);
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

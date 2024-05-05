#include "object.h"
#include "vm.h"

std::string Lox::Object::to_string() const
{
	return "";
}

Lox::ObjectString* Lox::ObjectString::allocate(const std::string& string)
{
	size_t hash = std::hash<std::string>()(string);
	auto iter = vm.strings.find(hash);
	if (iter != vm.strings.end())
	{
		return iter->second;
	}

	Lox::ObjectString* result = new Lox::ObjectString(string);
	result->next = vm.objects;
	vm.objects = result;

	vm.strings.insert({hash, result});

	return result;
}

const std::string& Lox::ObjectString::get_string() const
{
	return string;
}

std::string Lox::ObjectString::to_string() const
{
	return string;
}

Lox::ObjectString::ObjectString(const std::string& in_string)
	: string(in_string)
{
}

Lox::ObjectString::~ObjectString()
{
	size_t hash = std::hash<std::string>()(string);
	vm.strings.erase(hash);
}

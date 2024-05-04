#include "object.h"
#include "vm.h"

std::string Lox::Object::to_string() const
{
	return "";
}

std::string Lox::ObjectString::to_string() const
{
	return string;
}

template<typename T>
T* Lox::allocate_object()
{
	static_assert(std::is_base_of<Lox::Object, T>());

	T* result = new T();
	result->next = vm.objects;
	vm.objects = result;

	return result;
}
template Lox::ObjectString* Lox::allocate_object<Lox::ObjectString>();

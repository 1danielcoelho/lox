#include "object.h"

std::string Lox::Object::to_string() const
{
	return "";
}

std::string Lox::ObjectString::to_string() const
{
	return string;
}

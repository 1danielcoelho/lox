#pragma once

#include <string>
#include <variant>

namespace Lox
{
	class Callable;
	class NativeFunction;

	using Object = std::variant<std::nullptr_t, double, bool, std::string, NativeFunction*>;

	std::string to_string(const Object& variant);
	const Callable* as_callable(const Object& object);
}

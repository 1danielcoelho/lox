#pragma once

#include <variant>
#include <string>

namespace Lox
{
	using Object = std::variant<std::nullptr_t, double, bool, std::string>;
	std::string to_string(const Object& variant);
}
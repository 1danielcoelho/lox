#pragma once

#include "common.h"

#include <string>
#include <variant>

namespace Lox
{
	using Value = std::variant<bool, nullptr_t, f64>;

	std::string to_string(const Value& value);
}

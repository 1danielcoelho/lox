#pragma once

#include "common.h"

#include <memory>
#include <string>
#include <variant>

namespace Lox
{
	class Object;

	// TODO: I think we could just use a std::shared_ptr<Object> here but
	// I don't know where he's going with the garbage collector stuff so I'll just
	// pass around a naked ptr
	using Value = std::variant<bool, nullptr_t, f64, Object*>;

	bool values_equal(const Value& left, const Value& right);
	std::string to_string(const Value& value);
}

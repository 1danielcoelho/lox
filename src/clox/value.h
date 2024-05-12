#pragma once

#include "common.h"

#include <memory>
#include <string>
#include <variant>

namespace Lox
{
	class Object;
	class ObjectString;
	class ObjectFunction;
	class ObjectClosure;
	class ObjectNativeFunction;

	using Value = std::variant<bool, nullptr_t, f64, Object*>;

	bool is_number(const Lox::Value& val);
	bool is_bool(const Lox::Value& val);
	bool is_nil(const Lox::Value& val);
	bool is_object(const Lox::Value& val);
	bool is_string(const Lox::Value& val);
	bool is_function(const Lox::Value& val);
	bool is_native(const Lox::Value& val);
	bool is_closure(const Lox::Value& val);

	f64 as_number(const Lox::Value& val);
	bool as_bool(const Lox::Value& val);
	Object* as_object(const Lox::Value& val);
	ObjectString* as_string(const Lox::Value& val);
	ObjectFunction* as_function(const Lox::Value& val);
	ObjectNativeFunction* as_native(const Lox::Value& val);
	ObjectClosure* as_closure(const Lox::Value& val);

	bool values_equal(const Value& left, const Value& right);
	Lox::String to_string(const Value& value);
}	 // namespace Lox

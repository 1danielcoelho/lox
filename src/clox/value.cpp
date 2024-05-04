#include "value.h"
#include "object.h"

#include <format>

bool Lox::is_number(const Lox::Value& val)
{
	return std::holds_alternative<f64>(val);
}

bool Lox::is_bool(const Lox::Value& val)
{
	return std::holds_alternative<bool>(val);
}

bool Lox::is_nil(const Lox::Value& val)
{
	return std::holds_alternative<nullptr_t>(val);
}

bool Lox::is_object(const Lox::Value& val)
{
	return std::holds_alternative<Lox::Object*>(val);
}

bool Lox::is_string(const Lox::Value& val)
{
	if (is_object(val))
	{
		return as_string(val);
	}

	return false;
}

f64 Lox::as_number(const Lox::Value& val)
{
	return std::get<f64>(val);
}

bool Lox::as_bool(const Lox::Value& val)
{
	return std::get<bool>(val);
}

Lox::Object* Lox::as_object(const Lox::Value& val)
{
	return std::get<Lox::Object*>(val);
}

Lox::ObjectString* Lox::as_string(const Lox::Value& val)
{
	return dynamic_cast<Lox::ObjectString*>(as_object(val));
}

bool Lox::values_equal(const Lox::Value& left_val, const Lox::Value& right_val)
{
	const bool left_is_obj = is_object(left_val);
	const bool right_is_obj = is_object(right_val);
	if (left_is_obj != right_is_obj)
	{
		return false;
	}
	else if (left_is_obj)
	{
		Object* left = as_object(left_val);
		Object* right = as_object(right_val);

		// Both strings: Do actual string comparison
		ObjectString* left_str = dynamic_cast<ObjectString*>(left);
		ObjectString* right_str = dynamic_cast<ObjectString*>(right);
		if (left_str && right_str)
		{
			return left_str->string == right_str->string;
		}

		// Both some other kind of object: Pointer comparison
		return left == right;
	}
	else
	{
		// Just defer back to the std::variant overload of operator==
		return left_val == right_val;
	}
}

std::string Lox::to_string(const Lox::Value& variant)
{
	struct Visitor
	{
		std::string operator()(const std::string& s)
		{
			return s;
		}
		std::string operator()(double d)
		{
			return std::to_string(d);
		}
		std::string operator()(bool b)
		{
			return b ? "true" : "false";
		}
		std::string operator()([[maybe_unused]] std::nullptr_t n)
		{
			return "nil";
		}
		std::string operator()([[maybe_unused]] Object* o)
		{
			return o->to_string();
		}
	};

	return std::visit(Visitor(), variant);
}

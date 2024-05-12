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

bool Lox::is_function(const Lox::Value& val)
{
	if (is_object(val))
	{
		return as_function(val);
	}

	return false;
}

bool Lox::is_native(const Lox::Value& val)
{
	if (is_object(val))
	{
		return as_native(val);
	}

	return false;
}

bool Lox::is_closure(const Lox::Value& val)
{
	if (is_object(val))
	{
		return as_closure(val);
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
	Lox::Object* obj = as_object(val);
	return dynamic_cast<Lox::ObjectString*>(obj);
}

Lox::ObjectFunction* Lox::as_function(const Lox::Value& val)
{
	return dynamic_cast<Lox::ObjectFunction*>(as_object(val));
}

Lox::ObjectNativeFunction* Lox::as_native(const Lox::Value& val)
{
	return dynamic_cast<Lox::ObjectNativeFunction*>(as_object(val));
}

Lox::ObjectClosure* Lox::as_closure(const Lox::Value& val)
{
	return dynamic_cast<Lox::ObjectClosure*>(as_object(val));
}

bool Lox::values_equal(const Lox::Value& left_val, const Lox::Value& right_val)
{
	// TODO: I *think* we won't need this because since we intern strings we can compare
	// ObjectString via pointer too and so we could just rely on the operator== automatic
	// implementation of std::variant... I think we may need this later in the book though

	const bool left_is_obj = is_object(left_val);
	const bool right_is_obj = is_object(right_val);
	if (left_is_obj != right_is_obj)
	{
		return false;
	}
	else
	{
		// Just defer back to the std::variant overload of operator==
		return left_val == right_val;
	}
}

Lox::String Lox::to_string(const Lox::Value& variant)
{
	struct Visitor
	{
		Lox::String operator()(const Lox::String& s)
		{
			return s;
		}
		Lox::String operator()(double d)
		{
			return Lox::String{std::to_string(d)};
		}
		Lox::String operator()(bool b)
		{
			return b ? "true" : "false";
		}
		Lox::String operator()([[maybe_unused]] std::nullptr_t n)
		{
			return "nil";
		}
		Lox::String operator()([[maybe_unused]] Object* o)
		{
			return o->to_string();
		}
	};

	return std::visit(Visitor(), variant);
}

#include "object.h"

std::string Lox::to_string(const Lox::Object& variant)
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
			return "";
		}
	};

	return std::visit(Visitor(), variant);
}

const Lox::Callable* as_callable(const Lox::Object& object)
{
	struct Visitor
	{
		const Lox::Callable* operator()(const std::string& s)
		{
			return nullptr;
		}
		const Lox::Callable* operator()(double d)
		{
			return nullptr;
		}
		const Lox::Callable* operator()(bool b)
		{
			return nullptr;
		}
		const Lox::Callable* operator()([[maybe_unused]] std::nullptr_t n)
		{
			return nullptr;
		}
	};

	return std::visit(Visitor(), object);
}

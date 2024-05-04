#include "value.h"

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
			return "";
		}
	};

	return std::visit(Visitor(), variant);
}

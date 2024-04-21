#include "object.h"
#include "native_function.h"

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
		std::string operator()([[maybe_unused]] Lox::NativeFunction* func)
		{
			return "<native function at " + std::to_string((unsigned long long)func) + ">";
		}
	};

	return std::visit(Visitor(), variant);
}

const Lox::Callable* Lox::as_callable(const Lox::Object& object)
{
	struct Visitor
	{
		const Lox::Callable* operator()([[maybe_unused]] const std::string& s)
		{
			return nullptr;
		}
		const Lox::Callable* operator()([[maybe_unused]] double d)
		{
			return nullptr;
		}
		const Lox::Callable* operator()([[maybe_unused]] bool b)
		{
			return nullptr;
		}
		const Lox::Callable* operator()([[maybe_unused]] std::nullptr_t n)
		{
			return nullptr;
		}
		const Lox::Callable* operator()(Lox::NativeFunction* func)
		{
			return func;
		}
	};

	return std::visit(Visitor(), object);
}

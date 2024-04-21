#pragma once

#include "callable.h"
#include "object.h"

#include <functional>
#include <vector>

namespace Lox
{
	class Interpreter;

	class NativeFunction : public Callable
	{
	public:
		Lox::Object (*call_ptr)(Interpreter&, const std::vector<Lox::Object>&) = nullptr;
		int (*arity_ptr)() = nullptr;

	public:
		virtual Lox::Object call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const override;
		virtual int arity() const override;
	};

	NativeFunction* get_clock_function();
}

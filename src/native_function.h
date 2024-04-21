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
		virtual Lox::Object call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const = 0;
		virtual int arity() const = 0;
	};

	class ClockFunction : public NativeFunction
	{
		virtual Lox::Object call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const override;
		virtual int arity() const override;
	};
}

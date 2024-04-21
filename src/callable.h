#pragma once

#include "object.h"

#include <functional>
#include <vector>

namespace Lox
{
	class Interpreter;

	class Callable
	{
	public:
		virtual Lox::Object call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const = 0;
		virtual int arity() const = 0;
	};
}

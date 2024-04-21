#pragma once

#include "callable.h"

#include <memory>

namespace Lox
{
	struct FunctionStatement;

	class Function : public Callable
	{
	public:
		// TODO: Still not super clear who "owns" these statements, but for now it seems to be the
		// return value of Lox::parse(), so here we just point to them.
		//
		// It's also useful because this lets Function be copiable, which is important since we want
		// to put it inside Lox::Object
		FunctionStatement* declaration;

	public:
		virtual Lox::Object call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const override;
		virtual int arity() const override;
	};
}

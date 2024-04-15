#pragma once

#include "expression.h"
#include "object.h"

#include <optional>
#include <string>

namespace Lox
{
	// TODO: Should this be an internal class?
	// TODO: Need a const access visitor, as I don't think this needs to modify anything
	class Interpreter : public ExpressionVisitor
	{
	public:
		std::optional<Object> result;

	public:
		// TODO: This doesn't output anything?
		void interpret(Expression& expr);

	public:
		virtual void visit(Expression& expr) override;
		virtual void visit(LiteralExpression& expr) override;
		virtual void visit(GroupingExpression& expr) override;
		virtual void visit(UnaryExpression& expr) override;
		virtual void visit(BinaryExpression& expr) override;
	};
}

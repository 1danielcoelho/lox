#pragma once

#include "expression.h"

#include <optional>
#include <string>

namespace Lox
{
	class ASTPrinter : public ExpressionVisitor
	{
	public:
		// We can't have virtual templated methods so we just stash return types inside
		// the visitor instead
		std::optional<std::string> result;

	public:
		virtual void visit(Expression& expr) override;
		virtual void visit(LiteralExpression& expr) override;
		virtual void visit(GroupingExpression& expr) override;
		virtual void visit(UnaryExpression& expr) override;
		virtual void visit(BinaryExpression& expr) override;
	};
}

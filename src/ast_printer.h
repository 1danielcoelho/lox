#pragma once

#include "expression.h"

#include <optional>
#include <string>

namespace Lox
{
	// This class abuses the fact that one of the variants of Lox::Object is std::string to instead
	// stuff the stringified version of the expressions into Lox::Objects
	class ASTPrinter : public ExpressionVisitor
	{
	public:
		virtual std::optional<Object> visit(Expression& expr) override;
		virtual std::optional<Object> visit(LiteralExpression& expr) override;
		virtual std::optional<Object> visit(GroupingExpression& expr) override;
		virtual std::optional<Object> visit(UnaryExpression& expr) override;
		virtual std::optional<Object> visit(BinaryExpression& expr) override;
	};
}

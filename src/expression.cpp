#include "expression.h"

std::optional<Lox::Object> Lox::Expression::accept(ExpressionVisitor& visitor)
{
	return visitor.visit(*this);
}

std::optional<Lox::Object> Lox::LiteralExpression::accept(ExpressionVisitor& visitor)
{
	return visitor.visit(*this);
}

std::optional<Lox::Object> Lox::GroupingExpression::accept(ExpressionVisitor& visitor)
{
	return visitor.visit(*this);
}

std::optional<Lox::Object> Lox::UnaryExpression::accept(ExpressionVisitor& visitor)
{
	return visitor.visit(*this);
}

std::optional<Lox::Object> Lox::BinaryExpression::accept(ExpressionVisitor& visitor)
{
	return visitor.visit(*this);
}

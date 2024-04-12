#include "expression.h"

void Lox::Expression::accept(ExpressionVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::LiteralExpression::accept(ExpressionVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::GroupingExpression::accept(ExpressionVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::UnaryExpression::accept(ExpressionVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::BinaryExpression::accept(ExpressionVisitor& visitor)
{
	visitor.visit(*this);
}

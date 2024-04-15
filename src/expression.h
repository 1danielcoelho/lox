#pragma once

#include "token.h"

#include <memory>

namespace Lox
{
	class ExpressionVisitor;

	struct Expression
	{
		virtual ~Expression(){};
		virtual void accept(ExpressionVisitor& visitor);
	};

	struct LiteralExpression : public Expression
	{
		Object literal;

	public:
		virtual void accept(ExpressionVisitor& visitor) override;
	};

	struct GroupingExpression : public Expression
	{
		std::unique_ptr<Expression> expr;

	public:
		virtual void accept(ExpressionVisitor& visitor) override;
	};

	struct UnaryExpression : public Expression
	{
		Token op;
		std::unique_ptr<Expression> right;

	public:
		virtual void accept(ExpressionVisitor& visitor) override;
	};

	struct BinaryExpression : public Expression
	{
		std::unique_ptr<Expression> left;
		Token op;
		std::unique_ptr<Expression> right;

	public:
		virtual void accept(ExpressionVisitor& visitor) override;
	};

	class ExpressionVisitor
	{
	public:
		virtual void visit(Expression& expr) = 0;
		virtual void visit(LiteralExpression& expr) = 0;
		virtual void visit(GroupingExpression& expr) = 0;
		virtual void visit(UnaryExpression& expr) = 0;
		virtual void visit(BinaryExpression& expr) = 0;
	};
}	 // namespace Lox

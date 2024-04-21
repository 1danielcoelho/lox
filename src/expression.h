#pragma once

#include "token.h"

#include <memory>
#include <optional>
#include <vector>

namespace Lox
{
	class ExpressionVisitor;

	struct Expression
	{
		virtual ~Expression(){};
		virtual std::optional<Object> accept(ExpressionVisitor& visitor);
	};

	struct LiteralExpression : public Expression
	{
		Object literal;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct GroupingExpression : public Expression
	{
		std::unique_ptr<Expression> expr;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct UnaryExpression : public Expression
	{
		Token op;
		std::unique_ptr<Expression> right;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct BinaryExpression : public Expression
	{
		std::unique_ptr<Expression> left;
		Token op;
		std::unique_ptr<Expression> right;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct VariableExpression : public Expression
	{
		Token name;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct AssignmentExpression : public Expression
	{
		Token name;
		std::unique_ptr<Expression> value;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct LogicalExpression : public Expression
	{
		std::unique_ptr<Expression> left;
		Token op;
		std::unique_ptr<Expression> right;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	struct CallExpression : public Expression
	{
		std::unique_ptr<Expression> callee;
		Token paren;
		std::vector<std::unique_ptr<Expression>> arguments;

	public:
		virtual std::optional<Object> accept(ExpressionVisitor& visitor) override;
	};

	// TODO: We can't have templated virtual methods so we have a separate interface
	// depending on the return type.
	// Likely could be better but I'm not sure where the book is going with these just
	// yet
	class ExpressionVisitor
	{
	public:
		virtual std::optional<Object> visit(Expression& expr) = 0;
		virtual std::optional<Object> visit(LiteralExpression& expr) = 0;
		virtual std::optional<Object> visit(GroupingExpression& expr) = 0;
		virtual std::optional<Object> visit(UnaryExpression& expr) = 0;
		virtual std::optional<Object> visit(BinaryExpression& expr) = 0;
		virtual std::optional<Object> visit(VariableExpression& expr) = 0;
		virtual std::optional<Object> visit(AssignmentExpression& expr) = 0;
		virtual std::optional<Object> visit(LogicalExpression& expr) = 0;
		virtual std::optional<Object> visit(CallExpression& expr) = 0;
	};
}	 // namespace Lox

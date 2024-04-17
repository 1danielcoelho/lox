#pragma once

#include "expression.h"
#include "object.h"
#include "statement.h"

#include <optional>
#include <string>
#include <vector>

namespace Lox
{
	// TODO: Should this be an internal class?
	// TODO: Need a const access visitor, as I don't think this needs to modify anything
	class Interpreter
		: public ExpressionVisitor
		, public StatementVisitor
	{
	public:
		void interpret(const std::vector<std::unique_ptr<Statement>>& statements);

	public:
		virtual std::optional<Object> visit(Expression& expr) override;
		virtual std::optional<Object> visit(LiteralExpression& expr) override;
		virtual std::optional<Object> visit(GroupingExpression& expr) override;
		virtual std::optional<Object> visit(UnaryExpression& expr) override;
		virtual std::optional<Object> visit(BinaryExpression& expr) override;

		virtual void visit(Statement& statement) override;
		virtual void visit(ExpressionStatement& statement) override;
		virtual void visit(PrintStatement& statement) override;
	};
}

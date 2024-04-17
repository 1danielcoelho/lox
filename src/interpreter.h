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
		std::optional<Object> result;

	public:
		void interpret(const std::vector<std::unique_ptr<Statement>>& statements);

	public:
		virtual void visit(Expression& expr) override;
		virtual void visit(LiteralExpression& expr) override;
		virtual void visit(GroupingExpression& expr) override;
		virtual void visit(UnaryExpression& expr) override;
		virtual void visit(BinaryExpression& expr) override;

		virtual void visit(Statement& statement) override;
		virtual void visit(ExpressionStatement& statement) override;
		virtual void visit(PrintStatement& statement) override;
	};
}

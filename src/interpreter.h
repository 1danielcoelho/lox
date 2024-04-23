#pragma once

#include "environment.h"
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
		std::shared_ptr<Environment> global_environment;
		std::weak_ptr<Environment> current_environment;

	public:
		Interpreter();

		std::shared_ptr<Environment> get_global_environment() const;
		std::shared_ptr<Environment> get_current_environment() const;

		void interpret(const std::vector<std::unique_ptr<Statement>>& statements);

	public:
		virtual std::optional<Object> visit(Expression& expr) override;
		virtual std::optional<Object> visit(LiteralExpression& expr) override;
		virtual std::optional<Object> visit(GroupingExpression& expr) override;
		virtual std::optional<Object> visit(UnaryExpression& expr) override;
		virtual std::optional<Object> visit(BinaryExpression& expr) override;
		virtual std::optional<Object> visit(VariableExpression& expr) override;
		virtual std::optional<Object> visit(AssignmentExpression& expr) override;
		virtual std::optional<Object> visit(LogicalExpression& expr) override;
		virtual std::optional<Object> visit(CallExpression& expr) override;

		virtual void visit(Statement& statement) override;
		virtual void visit(ExpressionStatement& statement) override;
		virtual void visit(PrintStatement& statement) override;
		virtual void visit(VariableDeclarationStatement& statement) override;
		virtual void visit(BlockStatement& statement) override;
		virtual void visit(IfStatement& statement) override;
		virtual void visit(WhileStatement& statement) override;
		virtual void visit(FunctionStatement& statement) override;
		virtual void visit(ReturnStatement& statement) override;

	public:
		std::optional<Object> evaluate_expression(Expression& expr);
		void execute_statement(Statement& statement);
		void execute_block(std::vector<std::unique_ptr<Statement>>& statements, const std::shared_ptr<Environment>& environment);
	};
}	 // namespace Lox

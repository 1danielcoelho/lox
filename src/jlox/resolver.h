#pragma once

#include "environment.h"
#include "expression.h"
#include "interpreter.h"
#include "object.h"
#include "statement.h"

#include <vector>

namespace Lox
{
	enum class FunctionType : uint8_t
	{
		NONE,
		FUNCTION
	};

	class Resolver
		: public ExpressionVisitor
		, public StatementVisitor
	{
		Interpreter& interpreter;

		// The boolean represents whether or not we have finished resolving that variable's initializer
		std::vector<std::unordered_map<std::string, bool>> scopes;
		FunctionType current_function = FunctionType::NONE;

	public:
		Resolver(Interpreter& interpreter);

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
		void resolve(const std::vector<std::unique_ptr<Statement>>& statements);

	protected:
		void resolve(Statement& statement);
		void resolve(Expression& expression);
		void resolve_local(Expression& expression, const Token& name);
		void resolve_function(FunctionStatement& function, FunctionType function_type);

		void begin_scope();
		void end_scope();

		void declare(const Token& name);
		void define(const Token& name);
	};
}	 // namespace Lox

#pragma once

#include "expression.h"
#include "token.h"

#include <memory>
#include <vector>

namespace Lox
{
	class StatementVisitor;

	struct Statement
	{
		virtual ~Statement(){};
		virtual void accept(StatementVisitor& visitor);
	};

	struct ExpressionStatement : public Statement
	{
		std::unique_ptr<Expression> expression;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct PrintStatement : public Statement
	{
		std::unique_ptr<Expression> expression;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct VariableDeclarationStatement : public Statement
	{
		Token name;
		std::unique_ptr<Expression> initializer;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct BlockStatement : public Statement
	{
		std::vector<std::unique_ptr<Statement>> statements;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct IfStatement : public Statement
	{
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Statement> then_branch;
		std::unique_ptr<Statement> else_branch;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct WhileStatement : public Statement
	{
		std::unique_ptr<Expression> condition;
		std::unique_ptr<Statement> body;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct FunctionStatement : public Statement
	{
		Token name;
		std::vector<Token> params;
		std::vector<std::unique_ptr<Statement>> body;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	struct ReturnStatement : public Statement
	{
		Token keyword;
		std::unique_ptr<Expression> value;

	public:
		virtual void accept(StatementVisitor& visitor) override;
	};

	class StatementVisitor
	{
	public:
		virtual void visit(Statement& expr) = 0;
		virtual void visit(ExpressionStatement& expr) = 0;
		virtual void visit(PrintStatement& expr) = 0;
		virtual void visit(VariableDeclarationStatement& expr) = 0;
		virtual void visit(BlockStatement& expr) = 0;
		virtual void visit(IfStatement& expr) = 0;
		virtual void visit(WhileStatement& expr) = 0;
		virtual void visit(FunctionStatement& expr) = 0;
		virtual void visit(ReturnStatement& expr) = 0;
	};
}	 // namespace Lox

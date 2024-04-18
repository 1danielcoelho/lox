#pragma once

#include "expression.h"
#include "token.h"

#include <memory>

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

	class StatementVisitor
	{
	public:
		virtual void visit(Statement& expr) = 0;
		virtual void visit(ExpressionStatement& expr) = 0;
		virtual void visit(PrintStatement& expr) = 0;
		virtual void visit(VariableDeclarationStatement& expr) = 0;
	};
}	 // namespace Lox

#include "statement.h"

void Lox::Statement::accept(StatementVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::ExpressionStatement::accept(StatementVisitor& visitor)
{
	visitor.visit(*this);
}

void Lox::PrintStatement::accept(StatementVisitor& visitor)
{
	visitor.visit(*this);
}

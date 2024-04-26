#include "resolver.h"
#include "error.h"
#include "scoped_guard_value.h"

Lox::Resolver::Resolver(Lox::Interpreter& in_interpreter)
	: interpreter(in_interpreter)
{
}

std::optional<Lox::Object> Lox::Resolver::visit(Expression& expr)
{
	return expr.accept(*this);
}

std::optional<Lox::Object> Lox::Resolver::visit([[maybe_unused]] LiteralExpression& expr)
{
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(GroupingExpression& expr)
{
	resolve(*expr.expr);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(UnaryExpression& expr)
{
	resolve(*expr.right);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(BinaryExpression& expr)
{
	resolve(*expr.left);
	resolve(*expr.right);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(VariableExpression& expr)
{
	if (!scopes.empty())
	{
		auto iter = scopes.back().find(expr.name.lexeme);
		if (iter != scopes.back().end() && iter->second == false)
		{
			Lox::report_error(expr.name, "Can't read local variable in its own initializer");
		}
	}

	resolve_local(expr, expr.name);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(AssignmentExpression& expr)
{
	resolve(*expr.value);
	resolve_local(expr, expr.name);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(LogicalExpression& expr)
{
	resolve(*expr.left);
	resolve(*expr.right);
	return {};
}

std::optional<Lox::Object> Lox::Resolver::visit(CallExpression& expr)
{
	resolve(*expr.callee);
	for (const std::unique_ptr<Expression>& argument : expr.arguments)
	{
		resolve(*argument);
	}
	return {};
}

void Lox::Resolver::visit(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Resolver::visit(ExpressionStatement& statement)
{
	resolve(*statement.expression);
}

void Lox::Resolver::visit(PrintStatement& statement)
{
	resolve(*statement.expression);
}

void Lox::Resolver::visit(VariableDeclarationStatement& statement)
{
	declare(statement.name);
	if (statement.initializer)
	{
		resolve(*statement.initializer);
	}
	define(statement.name);
}

void Lox::Resolver::visit(BlockStatement& statement)
{
	begin_scope();
	resolve(statement.statements);
	end_scope();
}

void Lox::Resolver::visit(IfStatement& statement)
{
	resolve(*statement.condition);
	resolve(*statement.then_branch);
	if (statement.else_branch)
	{
		resolve(*statement.else_branch);
	}
}

void Lox::Resolver::visit(WhileStatement& statement)
{
	resolve(*statement.condition);
	resolve(*statement.body);
}

void Lox::Resolver::visit(FunctionStatement& statement)
{
	declare(statement.name);
	define(statement.name);

	resolve_function(statement, Lox::FunctionType::FUNCTION);
}

void Lox::Resolver::visit(ReturnStatement& statement)
{
	if (current_function == FunctionType::NONE)
	{
		Lox::report_error(statement.keyword, "Can't return from top-level code");
	}

	if (statement.value)
	{
		resolve(*statement.value);
	}
}

void Lox::Resolver::resolve(const std::vector<std::unique_ptr<Lox::Statement>>& statements)
{
	for (const std::unique_ptr<Lox::Statement>& statement : statements)
	{
		resolve(*statement);
	}
}

void Lox::Resolver::resolve(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Resolver::resolve(Expression& expression)
{
	expression.accept(*this);
}

void Lox::Resolver::resolve_local(Expression& expression, const Token& name)
{
	for (int i = (int)scopes.size() - 1; i >= 0; --i)
	{
		if (scopes[i].count(name.lexeme) > 0)
		{
			interpreter.resolve(expression, (int)(scopes.size() - 1 - i));
			return;
		}
	}
}

void Lox::Resolver::resolve_function(FunctionStatement& function, FunctionType function_type)
{
	ScopedGuardValue guard{current_function, function_type};

	begin_scope();
	for (const Token& param : function.params)
	{
		declare(param);
		define(param);
	}
	resolve(function.body);
	end_scope();
}

void Lox::Resolver::begin_scope()
{
	scopes.emplace_back();
}

void Lox::Resolver::end_scope()
{
	scopes.pop_back();
}

void Lox::Resolver::declare(const Token& name)
{
	if (scopes.empty())
	{
		return;
	}

	auto iter = scopes.back().insert({name.lexeme, false});
	bool inserted = iter.second;
	if (!inserted)
	{
		Lox::report_error(name, "A variable with this name already exists in this scope");
	}
}

void Lox::Resolver::define(const Token& name)
{
	if (scopes.empty())
	{
		return;
	}

	scopes.back().insert({name.lexeme, true});
}

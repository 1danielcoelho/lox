#include "interpreter.h"
#include "callable.h"
#include "error.h"
#include "expression.h"
#include "function.h"
#include "native_function.h"
#include "scoped_guard_value.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace InterpreterInternal
{
	using namespace Lox;

	bool is_truthy(const Object& obj)
	{
		if (std::holds_alternative<std::nullptr_t>(obj))
		{
			return false;
		}
		else if (std::holds_alternative<bool>(obj))
		{
			return std::get<bool>(obj);
		}

		// TODO: Maybe 0.0 should return false too?
		return true;
	}

	void check_number_operand(const Token& token, const Object& operand)
	{
		if (std::holds_alternative<double>(operand))
		{
			return;
		}

		throw RuntimeError{token, "Operand must be a number."};
	}

	void check_number_operands(const Token& token, const Object& left, const Object& right)
	{
		if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
		{
			return;
		}

		throw RuntimeError{token, "Both operands must be numbers."};
	}
}	 // namespace InterpreterInternal

Lox::Interpreter::Interpreter()
	: global_environment(std::make_unique<Environment>())
	, current_environment(global_environment)
{
	global_environment->define_variable("clock", Lox::get_clock_function());
}

std::shared_ptr<Lox::Environment> Lox::Interpreter::get_global_environment() const
{
	return global_environment;
}

std::shared_ptr<Lox::Environment> Lox::Interpreter::get_current_environment() const
{
	return current_environment.lock();
}

void Lox::Interpreter::interpret(const std::vector<std::unique_ptr<Statement>>& statements)
{
	using namespace InterpreterInternal;

	try
	{
		for (const std::unique_ptr<Statement>& statement : statements)
		{
			execute_statement(*statement);
		}
	}
	catch (const RuntimeError& e)
	{
		Lox::report_runtime_error(e.token, e.error_message);
	}
}

std::optional<Lox::Object> Lox::Interpreter::visit(Expression& expr)
{
	return expr.accept(*this);
}

std::optional<Lox::Object> Lox::Interpreter::visit(LiteralExpression& expr)
{
	return expr.literal;
}

std::optional<Lox::Object> Lox::Interpreter::visit(GroupingExpression& expr)
{
	return evaluate_expression(*expr.expr);
}

std::optional<Lox::Object> Lox::Interpreter::visit(UnaryExpression& expr)
{
	using namespace InterpreterInternal;

	std::optional<Object> right = evaluate_expression(*expr.right);

	switch (expr.op.type)
	{
		case TokenType::MINUS:
		{
			check_number_operand(expr.op, right.value());
			return -std::get<double>(right.value());
			break;
		}
		case TokenType::BANG:
		{
			return !is_truthy(right.value());
			break;
		}
		default:
		{
			assert(false);
			return {};
			break;
		}
	}
}

std::optional<Lox::Object> Lox::Interpreter::visit(BinaryExpression& expr)
{
	using namespace InterpreterInternal;

	std::optional<Object> left_optional = evaluate_expression(*expr.left);
	std::optional<Object> right_optional = evaluate_expression(*expr.right);
	Object& left = left_optional.value();
	Object& right = right_optional.value();

	switch (expr.op.type)
	{
		case TokenType::MINUS:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) - std::get<double>(right);
			break;
		}
		case TokenType::SLASH:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) / std::get<double>(right);
			break;
		}
		case TokenType::STAR:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) * std::get<double>(right);
			break;
		}
		case TokenType::PLUS:
		{
			if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
			{
				return std::get<double>(left) + std::get<double>(right);
			}
			else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
			{
				return std::get<std::string>(left) + std::get<std::string>(right);
			}
			else
			{
				throw RuntimeError{expr.op, "Operands must be either two numbers or two strings."};
			}
			break;
		}
		case TokenType::GREATER:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) > std::get<double>(right);
			break;
		}
		case TokenType::GREATER_EQUAL:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) >= std::get<double>(right);
			break;
		}
		case TokenType::LESS:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) < std::get<double>(right);
			break;
		}
		case TokenType::LESS_EQUAL:
		{
			check_number_operands(expr.op, left, right);
			return std::get<double>(left) <= std::get<double>(right);
			break;
		}
		case TokenType::BANG_EQUAL:
		{
			// TODO: Do these work?
			return (left != right);
			break;
		}
		case TokenType::EQUAL_EQUAL:
		{
			return (left == right);
			break;
		}
		default:
		{
			assert(false);
			return {};
			break;
		}
	}
}

std::optional<Lox::Object> Lox::Interpreter::visit(VariableExpression& expr)
{
	return lookup_variable(expr.name, expr);
}

std::optional<Lox::Object> Lox::Interpreter::visit(AssignmentExpression& expr)
{
	Lox::Object value = evaluate_expression(*expr.value).value();

	auto iter = locals.find(&expr);
	if (iter != locals.end())
	{
		get_current_environment()->assign_variable_at(iter->second, expr.name, value);
	}
	else
	{
		get_global_environment()->assign_variable(expr.name, value);
	}

	return value;
}

std::optional<Lox::Object> Lox::Interpreter::visit(LogicalExpression& expr)
{
	Lox::Object left = evaluate_expression(*expr.left).value();

	if (expr.op.type == TokenType::OR)
	{
		if (InterpreterInternal::is_truthy(left))
		{
			return left;
		}
	}
	else	// if expr.op.type == TokenType::AND
	{
		if (!InterpreterInternal::is_truthy(left))
		{
			return left;
		}
	}

	return evaluate_expression(*expr.right);
}

std::optional<Lox::Object> Lox::Interpreter::visit(CallExpression& expr)
{
	Lox::Object callee = evaluate_expression(*expr.callee).value();

	std::vector<Lox::Object> arguments;
	arguments.reserve(expr.arguments.size());
	for (const std::unique_ptr<Expression>& argument : expr.arguments)
	{
		arguments.push_back(evaluate_expression(*argument).value());
	}

	const Lox::Callable* function = Lox::as_callable(callee);
	if (!function)
	{
		throw RuntimeError{expr.paren, "Can only call functions and classes"};
	}

	if (arguments.size() != function->arity())
	{
		throw RuntimeError{
			expr.paren,
			"Expected " + std::to_string(function->arity()) + " arguments but got " + std::to_string(arguments.size()) + " instead"};
	}

	return function->call(*this, arguments);
}

void Lox::Interpreter::visit(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Interpreter::visit(ExpressionStatement& statement)
{
	// We don't do anything with the result here, that's the whole point
	evaluate_expression(*statement.expression);
}

void Lox::Interpreter::visit(PrintStatement& statement)
{
	std::optional<Object> result = evaluate_expression(*statement.expression);
	std::cout << Lox::to_string(result.value()) << std::endl;
}

void Lox::Interpreter::visit(VariableDeclarationStatement& statement)
{
	Lox::Object value = {nullptr};
	if (statement.initializer)
	{
		value = evaluate_expression(*statement.initializer).value();
	}

	current_environment.lock()->define_variable(statement.name.lexeme, value);
}

void Lox::Interpreter::visit(BlockStatement& statement)
{
	Environment* parent_environment = current_environment.lock().get();
	std::shared_ptr<Environment> block_environment = std::make_shared<Environment>(parent_environment);
	execute_block(statement.statements, block_environment);
}

void Lox::Interpreter::visit(IfStatement& statement)
{
	if (InterpreterInternal::is_truthy(evaluate_expression(*statement.condition).value()))
	{
		execute_statement(*statement.then_branch);
	}
	else if (statement.else_branch)
	{
		execute_statement(*statement.then_branch);
	}
}

void Lox::Interpreter::visit(WhileStatement& statement)
{
	while (InterpreterInternal::is_truthy(evaluate_expression(*statement.condition).value()))
	{
		execute_statement(*statement.body);
	}
}

void Lox::Interpreter::visit(FunctionStatement& statement)
{
	std::shared_ptr<Function> function = std::make_shared<Function>();
	function->declaration = &statement;	   // TODO: Ugh, hopefully this doesn't get reallocated I guess?
	function->closure = current_environment.lock();

	current_environment.lock()->define_variable(statement.name.lexeme, function);
}

void Lox::Interpreter::visit(ReturnStatement& statement)
{
	Lox::Object value = nullptr;
	if (statement.value)
	{
		value = evaluate_expression(*statement.value).value();
	}

	throw Return{value};
}

std::optional<Lox::Object> Lox::Interpreter::evaluate_expression(Expression& expr)
{
	return expr.accept(*this);
}

void Lox::Interpreter::execute_statement(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Interpreter::execute_block(std::vector<std::unique_ptr<Statement>>& statements, const std::shared_ptr<Environment>& environment)
{
	ScopedGuardValue guard{current_environment, std::weak_ptr<Environment>{environment}};

	for (const std::unique_ptr<Statement>& statement : statements)
	{
		execute_statement(*statement);
	}
}

void Lox::Interpreter::resolve(Lox::Expression& expr, int depth)
{
	locals.insert({&expr, depth});
}

std::optional<Lox::Object> Lox::Interpreter::lookup_variable(const Token& name, Expression& expr)
{
	auto iter = locals.find(&expr);
	if (iter != locals.end())
	{
		return get_current_environment()->get_variable_at(iter->second, name.lexeme);
	}
	else
	{
		return get_global_environment()->get_variable(name);
	}
}

#include "interpreter.h"
#include "error.h"
#include "expression.h"
#include "guard_value.h"

#include <cassert>
#include <iostream>
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
	, current_environment(global_environment.get())
{
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
	return current_environment->get_variable(expr.name);
}

std::optional<Lox::Object> Lox::Interpreter::visit(AssignmentExpression& expr)
{
	Lox::Object value = evaluate_expression(*expr.value).value();
	current_environment->assign_variable(expr.name, value);
	return value;
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

	current_environment->define_variable(statement.name.lexeme, value);
}

void Lox::Interpreter::visit(BlockStatement& statement)
{
	std::unique_ptr<Environment> block_environment = std::make_unique<Environment>(current_environment);
	execute_block(statement.statements, *block_environment);
}

std::optional<Lox::Object> Lox::Interpreter::evaluate_expression(Expression& expr)
{
	return expr.accept(*this);
}

void Lox::Interpreter::execute_statement(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Interpreter::execute_block(std::vector<std::unique_ptr<Statement>>& statements, Environment& environment)
{
	ScopedGuardValue guard{current_environment, &environment};

	for (const std::unique_ptr<Statement>& statement : statements)
	{
		execute_statement(*statement);
	}
}

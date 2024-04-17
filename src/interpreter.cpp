#include "interpreter.h"
#include "error.h"
#include "expression.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

namespace InterpreterInternal
{
	using namespace Lox;

	Object evaluate(Interpreter* interpreter_visitor, Expression& expr)
	{
		expr.accept(*interpreter_visitor);
		return interpreter_visitor->result.value();
	}

	void execute(Interpreter* interpreter_visitor, Statement& statement)
	{
		statement.accept(*interpreter_visitor);
	}

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

		// TODO: Maybe 0 should return false too?
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

void Lox::Interpreter::interpret(const std::vector<std::unique_ptr<Statement>>& statements)
{
	using namespace InterpreterInternal;

	try
	{
		for (const std::unique_ptr<Statement>& statement : statements)
		{
			execute(this, *statement);
		}
	}
	catch (const RuntimeError& e)
	{
		Lox::report_runtime_error(e.token, e.error_message);
	}
}

void Lox::Interpreter::visit(Expression& expr)
{
	expr.accept(*this);
}

void Lox::Interpreter::visit(LiteralExpression& expr)
{
	// TODO: I don't think any of this works because I'm reusing the same 'result' member over and over...
	result = expr.literal;
}

void Lox::Interpreter::visit(GroupingExpression& expr)
{
	result = InterpreterInternal::evaluate(this, *expr.expr);
}

void Lox::Interpreter::visit(UnaryExpression& expr)
{
	using namespace InterpreterInternal;

	Object right = evaluate(this, *expr.right);

	switch (expr.op.type)
	{
		case TokenType::MINUS:
		{
			check_number_operand(expr.op, right);
			result = -std::get<double>(right);
			break;
		}
		case TokenType::BANG:
		{
			result = !is_truthy(right);
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}
}

void Lox::Interpreter::visit(BinaryExpression& expr)
{
	using namespace InterpreterInternal;

	Object left = evaluate(this, *expr.left);
	Object right = evaluate(this, *expr.right);

	switch (expr.op.type)
	{
		case TokenType::MINUS:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) - std::get<double>(right);
			break;
		}
		case TokenType::SLASH:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) / std::get<double>(right);
			break;
		}
		case TokenType::STAR:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) * std::get<double>(right);
			break;
		}
		case TokenType::PLUS:
		{
			if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
			{
				result = std::get<double>(left) + std::get<double>(right);
			}
			else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
			{
				result = std::get<std::string>(left) + std::get<std::string>(right);
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
			result = std::get<double>(left) > std::get<double>(right);
			break;
		}
		case TokenType::GREATER_EQUAL:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) >= std::get<double>(right);
			break;
		}
		case TokenType::LESS:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) < std::get<double>(right);
			break;
		}
		case TokenType::LESS_EQUAL:
		{
			check_number_operands(expr.op, left, right);
			result = std::get<double>(left) <= std::get<double>(right);
			break;
		}
		case TokenType::BANG_EQUAL:
		{
			// TODO: Do these work?
			result = (left != right);
			break;
		}
		case TokenType::EQUAL_EQUAL:
		{
			result = (left == right);
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}
}

void Lox::Interpreter::visit(Statement& statement)
{
	statement.accept(*this);
}

void Lox::Interpreter::visit(ExpressionStatement& statement)
{
	InterpreterInternal::evaluate(this, *statement.expression);
}

void Lox::Interpreter::visit(PrintStatement& statement)
{
	InterpreterInternal::evaluate(this, *statement.expression);
	std::cout << Lox::to_string(*result) << std::endl;
}
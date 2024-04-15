#include "interpreter.h"
#include "expression.h"

#include <cassert>
#include <sstream>
#include <vector>
#include <iostream>

namespace InterpreterInternal
{
    Lox::Object evaluate(Lox::Interpreter* interpreter_visitor, Lox::Expression& expr)
    {
        expr.accept(*interpreter_visitor);
        return interpreter_visitor->result.value();
    }

    bool is_truthy(const Lox::Object& obj)
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

    struct RuntimeException
    {
        const Lox::Token& token;
        std::string error_message;
    };

    void check_number_operand(const Lox::Token& token, const Lox::Object& operand)
    {
        if (std::holds_alternative<double>(operand))
        {
            return;
        }

        throw RuntimeException{token, "Operand must be a number."};
    }

    void check_number_operands(const Lox::Token& token, const Lox::Object& left, const Lox::Object& right)
    {
        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            return;
        }

        throw RuntimeException{token, "Both operands must be numbers."};
    }
}

void Lox::Interpreter::interpret(Lox::Expression& expr)
{
    using namespace InterpreterInternal;

    try
    {
        Object value = evaluate(this, expr);
        std::cout << "interpreted '" << Lox::to_string(*result) << "'" << std::endl;
    }
    catch (const RuntimeException& e)
    {
        Lox::report_error(e);
    }
}

void Lox::Interpreter::visit(Expression& expr)
{
	expr.accept(*this);
}

void Lox::Interpreter::visit(LiteralExpression& expr)
{
    // I don't think any of this works because I'm reusing the same 'result' member over and over...
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
                throw RuntimeException{expr.op, "Operands must be either two numbers or two strings."};
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

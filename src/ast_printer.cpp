#include "ast_printer.h"

#include <cassert>
#include <sstream>
#include <vector>

namespace ASTPrinterInternal
{
	std::string parenthesize(Lox::ASTPrinter* printer_visitor, const std::string& name, const std::vector<Lox::Expression*>& expressions)
	{
		std::stringstream ss;
		ss << "(" << name;
		for (Lox::Expression* expr : expressions)
		{
			std::optional<Lox::Object> result = expr->accept(*printer_visitor);
			ss << " " << std::get<std::string>(result.value());
		}
		ss << ")";

		return ss.str();
	}
}

std::optional<Lox::Object> Lox::ASTPrinter::visit(Expression& expr)
{
	expr.accept(*this);
	return {};
}

std::optional<Lox::Object> Lox::ASTPrinter::visit(LiteralExpression& expr)
{
	if (std::holds_alternative<nullptr_t>(expr.literal))
	{
		return "nil";
	}

	return to_string(expr.literal);
}

std::optional<Lox::Object> Lox::ASTPrinter::visit(GroupingExpression& expr)
{
	return ASTPrinterInternal::parenthesize(this, "group", {expr.expr.get()});
}

std::optional<Lox::Object> Lox::ASTPrinter::visit(UnaryExpression& expr)
{
	return ASTPrinterInternal::parenthesize(this, expr.op.lexeme, {expr.right.get()});
}

std::optional<Lox::Object> Lox::ASTPrinter::visit(BinaryExpression& expr)
{
	return ASTPrinterInternal::parenthesize(this, expr.op.lexeme, {expr.left.get(), expr.right.get()});
}

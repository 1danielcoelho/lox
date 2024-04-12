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
			expr->accept(*printer_visitor);
			assert(printer_visitor->result.has_value());
			ss << printer_visitor->result.value();
		}
		ss << ")";

		return ss.str();
	}
}

void Lox::ASTPrinter::visit(Expression& expr)
{
	expr.accept(*this);
}

void Lox::ASTPrinter::visit(LiteralExpression& expr)
{
	if (std::holds_alternative<nullptr_t>(expr.literal))
	{
		result = "nil";
	}

	result = to_string(expr.literal);
}

void Lox::ASTPrinter::visit(GroupingExpression& expr)
{
	result = ASTPrinterInternal::parenthesize(this, "group", {&expr.expr});
}

void Lox::ASTPrinter::visit(UnaryExpression& expr)
{
	result = ASTPrinterInternal::parenthesize(this, expr.op.lexeme, {&expr.right});
}

void Lox::ASTPrinter::visit(BinaryExpression& expr)
{
	result = ASTPrinterInternal::parenthesize(this, expr.op.lexeme, {&expr.left, &expr.right});
}

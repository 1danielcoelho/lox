#include "token.h"

#include <sstream>

std::string Lox::to_string(const Lox::LiteralVariantType& variant)
{
	struct Visitor
	{
		std::string operator()(const std::string& s)
		{
			return s;
		}
		std::string operator()(double d)
		{
			return std::to_string(d);
		}
		std::string operator()(nullptr_t n)
		{
			return "";
		}
	};

	return std::visit(Visitor(), variant);
}

std::string Lox::Token::to_string() const
{
	std::stringstream ss;
	ss << (uint8_t)type << " " << lexeme << " ";	// TODO: Print the literal?
	return ss.str();
}

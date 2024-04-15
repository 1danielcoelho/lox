#include "token.h"

#include <sstream>

std::string Lox::Token::to_string() const
{
	std::stringstream ss;
	ss << (uint8_t)type << " " << lexeme << " ";	// TODO: Print the literal?
	return ss.str();
}

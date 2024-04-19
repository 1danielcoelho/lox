#include "environment.h"
#include "error.h"
#include "token.h"

void Lox::Environment::define_variable(const std::string& name, const Lox::Object& value)
{
	values.insert({name, value});
}

const Lox::Object& Lox::Environment::get_variable(const Lox::Token& token)
{
	auto iter = values.find(token.lexeme);
	if (iter == values.end())
	{
		throw Lox::RuntimeError{token, "Cannot get undefined variable '" + token.lexeme + "'"};
	}

	return iter->second;
}

void Lox::Environment::assign_variable(const Lox::Token& token, const Lox::Object& value)
{
	auto iter = values.find(token.lexeme);
	if (iter == values.end())
	{
		throw Lox::RuntimeError{token, "Cannot assign to undefined variable '" + token.lexeme + "'"};
	}

	values[token.lexeme] = value;
}

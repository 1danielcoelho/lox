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
	if (iter != values.end())
	{
		return iter->second;
	}

	if (std::shared_ptr<Environment> parent = enclosing_environment.lock())
	{
		return parent->get_variable(token);
	}

	throw Lox::RuntimeError{token, "Cannot get undefined variable '" + token.lexeme + "'"};
}

void Lox::Environment::assign_variable(const Lox::Token& token, const Lox::Object& value)
{
	auto iter = values.find(token.lexeme);
	if (iter != values.end())
	{
		values[token.lexeme] = value;
	}

	if (std::shared_ptr<Environment> parent = enclosing_environment.lock())
	{
		parent->assign_variable(token, value);
	}

	throw Lox::RuntimeError{token, "Cannot assign to undefined variable '" + token.lexeme + "'"};
}

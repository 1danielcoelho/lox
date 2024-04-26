#include "environment.h"
#include "error.h"
#include "token.h"

Lox::Environment::Environment(Lox::Environment* in_enclosing)
	: enclosing_environment(in_enclosing)
{
}

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

	if (enclosing_environment)
	{
		return enclosing_environment->get_variable(token);
	}

	throw Lox::RuntimeError{token, "Cannot get undefined variable '" + token.lexeme + "'"};
}

const Lox::Object& Lox::Environment::get_variable_at(int distance, const std::string& name)
{
	return ancestor(distance)->values.at(name);
}

void Lox::Environment::assign_variable(const Lox::Token& token, const Lox::Object& value)
{
	auto iter = values.find(token.lexeme);
	if (iter != values.end())
	{
		values[token.lexeme] = value;
		return;
	}

	if (enclosing_environment)
	{
		enclosing_environment->assign_variable(token, value);
		return;
	}

	throw Lox::RuntimeError{token, "Cannot assign to undefined variable '" + token.lexeme + "'"};
}

void Lox::Environment::assign_variable_at(int distance, const Lox::Token& name, const Lox::Object& value)
{
	ancestor(distance)->values.insert({name.lexeme, value});
}

Lox::Environment* Lox::Environment::ancestor(int distance)
{
	Lox::Environment* env = this;
	for (int i = 0; i < distance; ++i)
	{
		env = env->enclosing_environment;
	}

	return env;
}

#include "function.h"
#include "environment.h"
#include "error.h"
#include "interpreter.h"

#include <memory>

Lox::Object Lox::Function::call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const
{
	Environment* parent_environment = closure.get();
	std::shared_ptr<Environment> local_env = std::make_shared<Environment>(parent_environment);
	for (int i = 0; i < declaration->params.size(); ++i)
	{
		const Token& token = declaration->params[i];
		local_env->define_variable(token.lexeme, arguments[i]);
	}

	try
	{
		interpreter.execute_block(declaration->body, local_env);
	}
	catch (const Lox::Return& return_value)
	{
		return return_value.value;
	}

	// Return 'nil'
	return nullptr;
}

int Lox::Function::arity() const
{
	return static_cast<int>(declaration->params.size());
}

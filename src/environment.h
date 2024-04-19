#pragma once

#include "object.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Lox
{
	class Token;

	class Environment
	{
        // TODO: Have to handle these, he just yolos with Java pointers on the book
		std::weak_ptr<Environment> enclosing_environment;
		std::vector<std::unique_ptr<Environment>> child_environments;

		std::unordered_map<std::string, Lox::Object> values;

	public:
		void define_variable(const std::string& name, const Lox::Object& value);
		const Lox::Object& get_variable(const Lox::Token& token);
		void assign_variable(const Lox::Token& token, const Lox::Object& value);
	};
}

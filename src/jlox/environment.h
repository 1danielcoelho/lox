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
		Environment* enclosing_environment = nullptr;

		std::unordered_map<std::string, Lox::Object> values;

	public:
		Environment(Environment* in_enclosing = nullptr);

		void define_variable(const std::string& name, const Lox::Object& value);

		const Lox::Object& get_variable(const Lox::Token& token);
		const Lox::Object& get_variable_at(int distance, const std::string& name);

		void assign_variable(const Lox::Token& token, const Lox::Object& value);
		void assign_variable_at(int distance, const Lox::Token& name, const Lox::Object& value);

	private:
		Environment* ancestor(int distance);
	};
}

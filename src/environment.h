#pragma once

#include "object.h"

#include <string>
#include <unordered_map>

namespace Lox
{
	class Token;

	class Environment
	{
		std::unordered_map<std::string, Lox::Object> values;

	public:
		void define_variable(const std::string& name, const Lox::Object& value);
		const Lox::Object& get_variable(const Lox::Token& token);
		void assign_variable(const Lox::Token& token, const Lox::Object& value);
	};
}

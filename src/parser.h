#pragma once

#include "statement.h"
#include "token.h"

#include <memory>
#include <vector>

namespace Lox
{
	std::vector<Statement> parse(const std::vector<Token>& tokens);
}

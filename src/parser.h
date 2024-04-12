#pragma once

#include "expression.h"
#include "token.h"

#include <memory>
#include <vector>

namespace Lox
{
	std::unique_ptr<Expression> parse(const std::vector<Token>& tokens);
}

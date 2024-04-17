#pragma once

#include "statement.h"
#include "token.h"

#include <memory>
#include <vector>

namespace Lox
{
	std::vector<std::unique_ptr<Statement>> parse(const std::vector<Token>& tokens);
}

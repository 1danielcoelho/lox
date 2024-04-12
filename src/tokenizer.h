#pragma once

#include "token.h"

#include <string>
#include <vector>

namespace Lox
{
	std::vector<Token> tokenize(const std::string& text);
}

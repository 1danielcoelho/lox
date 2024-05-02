#include "compiler.h"
#include "scanner.h"

#include <format>
#include <iomanip>
#include <iostream>

void Lox::compile(const char* source)
{
	Lox::init_scanner(source);

	int line = -1;
	while (true)
	{
		Token token = scan_token();
		if (token.line != line)
		{
			std::cout << std::format("{:4} ", token.line);
			line = token.line;
		}
		else
		{
			std::cout << "   | ";
		}

		std::cout << std::format("{:2} '{:{}}'", (int)token.type, token.start, token.length) << std::endl;

		if (token.type == TokenType::EOF_)
		{
			break;
		}
	}
}

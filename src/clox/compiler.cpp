#include "compiler.h"
#include "scanner.h"

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
			std::cout << std::setw(4) << token.line << " ";
			line = token.line;
		}
        else
        {
			std::cout << "   | ";
        }
	}
}

#include "ast_printer.h"
#include "error.h"
#include "expression.h"
#include "interpreter.h"
#include "parser.h"
#include "resolver.h"
#include "tokenizer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

int run(const std::string& source)
{
	std::vector<Lox::Token> tokens = Lox::tokenize(source);
	std::vector<std::unique_ptr<Lox::Statement>> statements = Lox::parse(tokens);
	if (Lox::had_error())
	{
		return Lox::DATAERR;
	}

	static Lox::Interpreter interpreter;

	Lox::Resolver resolver{interpreter};
	resolver.resolve(statements);
	if (Lox::had_error())
	{
		return Lox::DATAERR;
	}

	interpreter.interpret(statements);

	if (Lox::had_error())
	{
		return Lox::DATAERR;
	}
	else if (Lox::had_runtime_error())
	{
		return Lox::SOFTWARE;
	}
	return Lox::SUCCESS;
}

int run_file(const char* arg)
{
	fs::path path{arg};
	if (!fs::is_regular_file(path))
	{
		return Lox::NOINPUT;
	}

	std::ifstream file_stream{path};
	if (!file_stream.is_open())
	{
		return Lox::IOERR;
	}

	std::stringstream sstream;
	sstream << file_stream.rdbuf();

	std::string file_contents;
	file_contents = sstream.str();

	file_stream.close();

	return run(file_contents);
}

int run_prompt()
{
	std::string input;
	while (true)
	{
		std::cout << "> ";

		if (!std::getline(std::cin, input))
		{
			break;
		}

		run(input);
		Lox::clear_error();
	}

	return Lox::SUCCESS;
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cout << "Use one or no argument" << std::endl;

		return Lox::USAGE;
	}
	else if (argc == 2)
	{
		// argv[0] is the file name of the executable
		return run_file(argv[1]);
	}
	else
	{
		return run_prompt();
	}
}

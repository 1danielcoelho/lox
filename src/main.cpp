#include "ast_printer.h"
#include "error.h"
#include "expression.h"
#include "interpreter.h"
#include "parser.h"
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
		return Lox::ERROR_CODE_DATAERR;
	}

	static Lox::Interpreter interpreter;
	interpreter.interpret(statements);

	if (Lox::had_error())
	{
		return Lox::ERROR_CODE_DATAERR;
	}
	else if (Lox::had_runtime_error())
	{
		return Lox::ERROR_CODE_SOFTWARE;
	}
	return Lox::ERROR_CODE_SUCCESS;
}

int run_file(const char* arg)
{
	fs::path path{arg};
	if (!fs::is_regular_file(path))
	{
		return Lox::ERROR_CODE_NOINPUT;
	}

	std::ifstream file_stream{path};
	if (!file_stream.is_open())
	{
		return Lox::ERROR_CODE_IOERR;
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

	return Lox::ERROR_CODE_SUCCESS;
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cout << "Use one or no argument" << std::endl;

		return Lox::ERROR_CODE_USAGE;
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

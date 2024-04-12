#include "error.h"
#include "tokenizer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

int run(const std::string& source)
{
	std::cout << "run: " << source << std::endl;

	std::vector<Lox::Token> tokens = Lox::tokenize(source);

	return Lox::had_error() ? Lox::ERROR_CODE_DATAERR : Lox::ERROR_CODE_SUCCESS;
}

int run_file(const char* arg)
{
	std::cout << "run_file: " << arg << std::endl;

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
	std::cout << "run_prompt" << std::endl;

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

		// Wrong usage error code: https://man.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html
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

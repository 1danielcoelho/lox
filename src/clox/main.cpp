#include "chunk.h"
#include "vm.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace Lox
{
	// https://man.freebsd.org/cgi/man.cgi?query=sysexits&apropos=0&sektion=0&manpath=FreeBSD+4.3-RELEASE&format=html
	enum ErrorCodes
	{
		ERROR_CODE_SUCCESS = 0,
		ERROR_CODE_USAGE = 64,
		ERROR_CODE_DATAERR = 65,
		ERROR_CODE_NOINPUT = 66,
		ERROR_CODE_SOFTWARE = 70,
		ERROR_CODE_IOERR = 74,
	};
}

Lox::String read_file(const char* path)
{
	fs::path fs_path{path};
	if (!fs::is_regular_file(fs_path))
	{
		std::cerr << "Could not find file '" << fs_path << "'" << std::endl;
		exit(Lox::ERROR_CODE_NOINPUT);
	}

	std::ifstream file_stream{fs_path};
	if (!file_stream.is_open())
	{
		std::cerr << "Could not open file '" << fs_path << "'" << std::endl;
		exit(Lox::ERROR_CODE_IOERR);
	}

	std::stringstream sstream;
	sstream << file_stream.rdbuf();

	Lox::String file_contents;
	file_contents = sstream.str();

	file_stream.close();

	return file_contents;
}

void run_file(const char* path)
{
	Lox::String source = read_file(path);
	Lox::InterpretResult result = Lox::interpret(source.c_str());

	if (result == Lox::InterpretResult::COMPILE_ERROR)
	{
		exit(Lox::ERROR_CODE_DATAERR);
	}

	if (result == Lox::InterpretResult::RUNTIME_ERROR)
	{
		exit(Lox::ERROR_CODE_SOFTWARE);
	}
}

void repl()
{
	Lox::String input;
	while (true)
	{
		std::cout << "> ";

		if (!std::getline(std::cin, input))
		{
			break;
		}

		Lox::interpret(input.c_str());
	}
}

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[])
{
	Lox::init_VM();

	Lox::Chunk c;

	if (argc == 1)
	{
		repl();
	}
	else if (argc == 2)
	{
		run_file(argv[1]);
	}
	else
	{
		std::cerr << "Usage: clox [path]" << std::endl;
		exit(Lox::ERROR_CODE_USAGE);
	}

	Lox::free_VM();
	return Lox::ERROR_CODE_SUCCESS;
}

#include "ast_printer.h"
#include "error.h"
#include "expression.h"
#include "tokenizer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

int run(const std::string& source)
{
	std::vector<Lox::Token> tokens = Lox::tokenize(source);

	return Lox::had_error() ? Lox::ERROR_CODE_DATAERR : Lox::ERROR_CODE_SUCCESS;
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
	using namespace Lox;

	std::unique_ptr<BinaryExpression> b = std::make_unique<BinaryExpression>();
	std::unique_ptr<UnaryExpression> u = std::make_unique<UnaryExpression>();
	std::unique_ptr<GroupingExpression> g = std::make_unique<GroupingExpression>();
	std::unique_ptr<LiteralExpression> l1 = std::make_unique<LiteralExpression>();
	std::unique_ptr<LiteralExpression> l2 = std::make_unique<LiteralExpression>();

	l1->literal = 123.0;
	l2->literal = 45.67;
	u->op = Token{TokenType::MINUS, "-", nullptr, 1};
	b->op = Token{TokenType::STAR, "*", nullptr, 1};

	u->right = std::move(l1);
	g->expr = std::move(l2);
	b->left = std::move(u);
	b->right = std::move(g);

	Lox::ASTPrinter printer;
	printer.visit(*b);
	std::cout << "test: " << printer.result.value() << std::endl;

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

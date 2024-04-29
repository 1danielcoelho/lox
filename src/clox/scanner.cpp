#include "scanner.h"

namespace ScannerImpl
{
	struct Scanner
	{
		const char* start = nullptr;
		const char* current = nullptr;
		int line = 0;
	};

	Scanner scanner;
}

void Lox::init_scanner(const char* source)
{
	using namespace ScannerImpl;

	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

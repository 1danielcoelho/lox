#include "native_function.h"

#include <chrono>

Lox::Object Lox::ClockFunction::call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const
{
	std::chrono::time_point<std::chrono::system_clock> time_point = std::chrono::system_clock::now();
	std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch());
	return static_cast<double>(millis.count() / 1000.0);
}

int Lox::ClockFunction::arity() const
{
	return 0;
}

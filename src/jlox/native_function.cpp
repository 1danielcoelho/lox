#include "native_function.h"

#include <cassert>
#include <chrono>
#include <memory>

Lox::Object Lox::NativeFunction::call(Interpreter& interpreter, const std::vector<Lox::Object>& arguments) const
{
	assert(call_ptr);
	return call_ptr(interpreter, arguments);
}

int Lox::NativeFunction::arity() const
{
	assert(arity_ptr);
	return arity_ptr();
}

Lox::NativeFunction* Lox::get_clock_function()
{
	static std::unique_ptr<Lox::NativeFunction> func;
	if (!func)
	{
		func = std::make_unique<NativeFunction>();
		func->call_ptr = []([[maybe_unused]] Interpreter& interpreter, [[maybe_unused]] const std::vector<Lox::Object>& arguments) -> Lox::Object
		{
			using namespace std::chrono;
			time_point<system_clock> time_point = system_clock::now();
			milliseconds millis = duration_cast<milliseconds>(time_point.time_since_epoch());
			return static_cast<double>(millis.count() / 1000.0);
		};
		func->arity_ptr = []() -> int
		{
			return 0;
		};
	}

	return func.get();
}

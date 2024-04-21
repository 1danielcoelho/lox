#pragma once

#include <memory>
#include <string>
#include <variant>

namespace Lox
{
	class Callable;
	class NativeFunction;
	class Function;

	// TODO: Still not exactly sure if std::shared_ptr<Function> is the right thing here...
	// I don't want slicing though, and Tokens have Object members and should be copiable, so that's what we have for now
	using Object = std::variant<std::nullptr_t, double, bool, std::string, NativeFunction*, std::shared_ptr<Function>>;

	std::string to_string(const Object& variant);
	const Callable* as_callable(const Object& object);
}

#pragma once

#include "common.h"

#include <string>

namespace Lox
{
	class Object
	{
	public:
		virtual ~Object(){};
		virtual std::string to_string() const;
	};

	class ObjectString : public Object
	{
	public:
		std::string string;

	public:
		virtual std::string to_string() const override;
	};

	// TODO: There was supposed to be an "allocate_object" function here but we haven't needed it yet
}

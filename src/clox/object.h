#pragma once

#include "common.h"

#include <string>

namespace Lox
{
	class Object
	{
	public:
		Object* next = nullptr;

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

	template<typename T>
	T* allocate_object();
}

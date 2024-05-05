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

	protected:
		Object(){};
	};

	class ObjectString : public Object
	{
	public:
		// Custom allocation function as these are garbage collected/interned
		static ObjectString* allocate(const std::string& string);

	public:
		virtual ~ObjectString() override;
		virtual std::string to_string() const override;

		const std::string& get_string() const;

	private:
		std::string string;

		ObjectString(const std::string& string);
	};
}	 // namespace Lox

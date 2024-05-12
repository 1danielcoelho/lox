#pragma once

#include "chunk.h"
#include "common.h"

#include <string>

namespace Lox
{
	class Object
	{
	public:
		bool is_marked = false;
		Object* next = nullptr;

	public:
		virtual ~Object(){};
		virtual void free() = 0;	// So that we can free an instance polymorphically, likely a bad idea...
		virtual Lox::String to_string() const;
	};

	class ObjectString : public Object
	{
	public:
		// Custom allocation as these are garbage collected/interned.
		// There is likely a cleaner way of doing this...
		static ObjectString* allocate(const Lox::String& string);
		static void free(ObjectString* instance);

		ObjectString(const Lox::String& string);
		virtual ~ObjectString() override;
		virtual void free() override;

		virtual Lox::String to_string() const override;
		const Lox::String& get_string() const;

	private:
		Lox::String string;
	};

	class ObjectFunction : public Object
	{
	public:
		i32 arity = 0;
		i32 upvalue_count = 0;
		Chunk chunk;
		ObjectString* name;

	public:
		static ObjectFunction* allocate();
		static void free(ObjectFunction* instance);

		virtual void free() override;

		virtual Lox::String to_string() const override;
	};

	class ObjectUpvalue : public Object
	{
	public:
		Value* location = nullptr;
		Value closed;
		ObjectUpvalue* next_upvalue = nullptr;

	public:
		static ObjectUpvalue* allocate(Value* slot);
		static void free(ObjectUpvalue* instance);

		ObjectUpvalue(Value* slot);
		virtual void free() override;

		virtual Lox::String to_string() const override;
	};

	class ObjectClosure : public Object
	{
	public:
		ObjectFunction* function;
		Lox::Vec<ObjectUpvalue*> upvalues;

	public:
		static ObjectClosure* allocate(ObjectFunction* function);
		static void free(ObjectClosure* instance);

		ObjectClosure(ObjectFunction* function);
		virtual void free() override;

		virtual Lox::String to_string() const override;
	};

	using NativeFn = Value (*)(i32 arg_count, Value* args);
	class ObjectNativeFunction : public Object
	{
	public:
		NativeFn function;

	public:
		static ObjectNativeFunction* allocate(NativeFn function);
		static void free(ObjectNativeFunction* instance);

		ObjectNativeFunction(NativeFn function);
		virtual void free() override;

		virtual Lox::String to_string() const override;
	};
}	 // namespace Lox

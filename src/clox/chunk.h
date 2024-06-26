#pragma once

#include "common.h"
#include "value.h"

#include <vector>

namespace Lox
{
	enum class Op : u8
	{
		CONSTANT,
		NIL,
		TRUE,
		FALSE,
		POP,
		GET_LOCAL,
		SET_LOCAL,
		GET_GLOBAL,
		DEFINE_GLOBAL,
		SET_GLOBAL,
		GET_UPVALUE,
		SET_UPVALUE,
		GET_PROPERTY,
		SET_PROPERTY,
		GET_SUPER,
		EQUAL,
		GREATER,
		LESS,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		NOT,
		NEGATE,
		PRINT,
		JUMP,
		JUMP_IF_FALSE,
		LOOP,
		CALL,
		INVOKE,
		SUPER_INVOKE,
		CLOSURE,
		CLOSE_UPVALUE,
		RETURN,
		CLASS,
		INHERIT,
		METHOD
	};

	class Chunk
	{
	public:
		Lox::Vec<u8> code;
		Lox::Vec<u32> lines;
		Lox::Vec<Value> constants;

	public:
		void disassemble_chunk(const char* chunk_name) const;
		i32 disassemble_instruction(i32 offset) const;

		void write_chunk(u8 byte, u32 line);
		i32 add_constant(Value value);

	private:
		i32 print_simple_instruction(const char* op_name, i32 offset) const;
		i32 print_constant_instruction(const char* op_name, i32 offset) const;
		i32 print_byte_instruction(const char* op_name, i32 offset) const;
		i32 print_jump_instruction(const char* op_name, i32 sign, i32 offset) const;
		i32 print_invoke_instruction(const char* op_name, i32 offset) const;
	};
}	 // namespace Lox

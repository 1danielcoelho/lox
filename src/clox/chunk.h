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
		CLOSURE,
		RETURN
	};

	class Chunk
	{
	public:
		std::vector<u8> code;
		std::vector<u32> lines;
		std::vector<Value> constants;

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
	};
}	 // namespace Lox

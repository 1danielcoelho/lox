#include "chunk.h"
#include "object.h"

#include <cassert>
#include <format>
#include <iomanip>
#include <iostream>
#include <string>

void Lox::Chunk::disassemble_chunk(const char* chunk_name) const
{
	std::cout << "== " << chunk_name << " ==" << std::endl;

	for (i32 offset = 0; offset < (i32)code.size();)
	{
		offset = disassemble_instruction(offset);
	}
}

i32 Lox::Chunk::disassemble_instruction(i32 offset) const
{
	std::cout << std::setw(4) << std::setfill('0') << offset << " ";

	Lox::Op instruction = static_cast<Lox::Op>(code[offset]);

	std::string line_number_str = (offset > 0 && lines[offset] == lines[offset - 1]) ? "    " : std::to_string(lines[offset]);
	std::cout << std::setw(4) << line_number_str << " ";
	switch (instruction)
	{
		case Lox::Op::CONSTANT:
		{
			return print_constant_instruction("CONSTANT", offset);
			break;
		}
		case Lox::Op::NIL:
		{
			return print_simple_instruction("NIL", offset);
			break;
		}
		case Lox::Op::TRUE:
		{
			return print_simple_instruction("TRUE", offset);
			break;
		}
		case Lox::Op::FALSE:
		{
			return print_simple_instruction("FALSE", offset);
			break;
		}
		case Lox::Op::POP:
		{
			return print_simple_instruction("POP", offset);
			break;
		}
		case Lox::Op::GET_LOCAL:
		{
			return print_byte_instruction("GET_LOCAL", offset);
			break;
		}
		case Lox::Op::SET_LOCAL:
		{
			return print_byte_instruction("SET_LOCAL", offset);
			break;
		}
		case Lox::Op::GET_GLOBAL:
		{
			return print_constant_instruction("GET_GLOBAL", offset);
			break;
		}
		case Lox::Op::DEFINE_GLOBAL:
		{
			return print_constant_instruction("DEFINE_GLOBAL", offset);
			break;
		}
		case Lox::Op::SET_GLOBAL:
		{
			return print_constant_instruction("SET_GLOBAL", offset);
			break;
		}
		case Lox::Op::GET_UPVALUE:
		{
			return print_byte_instruction("GET_UPVALUE", offset);
			break;
		}
		case Lox::Op::SET_UPVALUE:
		{
			return print_byte_instruction("SET_UPVALUE", offset);
			break;
		}
		case Lox::Op::EQUAL:
		{
			return print_simple_instruction("EQUAL", offset);
			break;
		}
		case Lox::Op::GREATER:
		{
			return print_simple_instruction("GREATER", offset);
			break;
		}
		case Lox::Op::LESS:
		{
			return print_simple_instruction("LESS", offset);
			break;
		}
		case Lox::Op::ADD:
		{
			return print_simple_instruction("ADD", offset);
			break;
		}
		case Lox::Op::SUBTRACT:
		{
			return print_simple_instruction("SUBTRACT", offset);
			break;
		}
		case Lox::Op::MULTIPLY:
		{
			return print_simple_instruction("MULTIPLY", offset);
			break;
		}
		case Lox::Op::DIVIDE:
		{
			return print_simple_instruction("DIVIDE", offset);
			break;
		}
		case Lox::Op::NOT:
		{
			return print_simple_instruction("NOT", offset);
			break;
		}
		case Lox::Op::NEGATE:
		{
			return print_simple_instruction("NEGATE", offset);
			break;
		}
		case Lox::Op::PRINT:
		{
			return print_simple_instruction("PRINT", offset);
			break;
		}
		case Lox::Op::JUMP:
		{
			return print_jump_instruction("JUMP", 1, offset);
			break;
		}
		case Lox::Op::JUMP_IF_FALSE:
		{
			return print_jump_instruction("JUMP_IF_FALSE", 1, offset);
			break;
		}
		case Lox::Op::LOOP:
		{
			return print_jump_instruction("LOOP", -1, offset);
			break;
		}
		case Lox::Op::CALL:
		{
			return print_byte_instruction("CALL", offset);
		}
		case Lox::Op::CLOSURE:
		{
			offset++;
			u8 const_index = code[offset++];
			std::cout << std::format("CLOSURE {} {}", const_index, to_string(constants[const_index])) << std::endl;

			ObjectFunction* function = as_function(constants[const_index]);
			for (i32 j = 0; j < function->upvalue_count; ++j)
			{
				i32 is_local = code[offset++];
				i32 index = code[offset++];

				std::cout << std::format("{:04}                {} {}", offset - 2, is_local ? "local" : "upvalue", index) << std::endl;
			}

			return offset;
		}
		case Lox::Op::RETURN:
		{
			return print_simple_instruction("RETURN", offset);
			break;
		}
		default:
		{
			std::cout << "Unknown opcode " << (u8)instruction << std::endl;
			return offset + 1;
		}
	}
}

void Lox::Chunk::write_chunk(u8 byte, u32 line)
{
	code.push_back(byte);
	lines.push_back(line);
}

i32 Lox::Chunk::add_constant(Value value)
{
	constants.push_back(value);
	return (i32)(constants.size() - 1);
}

i32 Lox::Chunk::print_simple_instruction(const char* op_name, i32 offset) const
{
	std::cout << op_name << std::endl;
	return offset + 1;
}

i32 Lox::Chunk::print_constant_instruction(const char* op_name, i32 offset) const
{
	u8 constant_index = code[offset + 1];
	std::cout << op_name << " ";
	std::cout << std::to_string(constant_index) << " '";
	std::cout << to_string(constants[constant_index]) << "'" << std::endl;
	return offset + 2;
}

i32 Lox::Chunk::print_byte_instruction(const char* op_name, i32 offset) const
{
	u8 slot = code[offset + 1];
	std::cout << std::format("{} {}", op_name, slot) << std::endl;
	return offset + 2;
}

i32 Lox::Chunk::print_jump_instruction(const char* op_name, i32 sign, i32 offset) const
{
	u16 jump = (u16)(code[offset + 1] << 8);
	jump |= code[offset + 2];

	std::cout << std::format("{} {} -> {}", op_name, offset, offset + 3 + sign * jump) << std::endl;
	return offset + 3;
}

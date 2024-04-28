#include "chunk.h"

#include <cassert>
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
		case Lox::Op::RETURN:
		{
			return print_simple_instruction("RETURN", offset);
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
		case Lox::Op::CONSTANT:
		{
			return print_constant_instruction("CONSTANT", offset);
			break;
		}
		case Lox::Op::NEGATE:
		{
			return print_simple_instruction("NEGATE", offset);
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
	std::cout << constants[constant_index] << "'" << std::endl;
	return offset + 2;
}

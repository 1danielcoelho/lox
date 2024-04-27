#include "chunk.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[])
{
	Lox::Chunk c;

	i32 constant_index = c.add_constant(1.2);
	c.write_chunk((u8)Lox::Op::CONSTANT, 123);
	c.write_chunk((u8)constant_index, 123);

	c.write_chunk((u8)Lox::Op::RETURN, 123);

	c.disassemble_chunk("test chunk");

	return 0;
}

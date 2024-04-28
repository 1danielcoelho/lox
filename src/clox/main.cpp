#include "chunk.h"
#include "vm.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[])
{
	Lox::initVM();

	Lox::Chunk c;

	i32 constant_index = c.add_constant(1.2);
	c.write_chunk((u8)Lox::Op::CONSTANT, 123);
	c.write_chunk((u8)constant_index, 123);

	constant_index = c.add_constant(3.4);
	c.write_chunk((u8)Lox::Op::CONSTANT, 123);
	c.write_chunk((u8)constant_index, 123);

	c.write_chunk((u8)Lox::Op::ADD, 123);

	constant_index = c.add_constant(5.6);
	c.write_chunk((u8)Lox::Op::CONSTANT, 123);
	c.write_chunk((u8)constant_index, 123);

	c.write_chunk((u8)Lox::Op::DIVIDE, 123);

	c.write_chunk((u8)Lox::Op::NEGATE, 123);

	c.write_chunk((u8)Lox::Op::RETURN, 123);

	Lox::interpret(c);
	Lox::freeVM();
	return 0;
}

#include "target/iris16/iris.h"

namespace iris16 {
	DecodedInstruction::DecodedInstruction() { }

	void DecodedInstruction::decode(raw_instruction input) {
#define X(title, mask, shift, type, is_register, post) \
		_ ## post = iris::decodeBits<raw_instruction, type, mask, shift>(input);
#include "target/iris16/instruction.def"
#undef X
	}
}

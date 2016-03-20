#ifndef _TARGET_IRIS32_IRIS_H
#define _TARGET_IRIS32_IRIS_H
#include "iris_base.h"
#include "Core.h"
#include <cstdint>

typedef int64_t dword;
typedef int32_t word;
typedef int16_t hword;

namespace iris32 {
	enum ArchitectureConstants  {
		RegisterCount = 256,
		AddressMax = 268435456,
		InstructionPointerIndex = RegisterCount - 1,
		LinkRegisterIndex = RegisterCount - 2,
		StackPointerIndex = RegisterCount - 3,
		CallPointerIndex = RegisterCount - 4,
		InternalTemporaryCount = 8,
	};
	enum {
		GroupMask = 0b00000111,
		RestMask = ~GroupMask,
	};
	class MemoryController {
		public:
			MemoryController(word memSize);
			~MemoryController();
			word read(word address);
			void write(word address, word value);
			void install(std::istream& stream);
			void dump(std::ostream& stream);
		private:
			word memorySize;
			word* memory;
	};
	enum InstructionGroup {
#define X(e, __) e ,
#include "iris32_groups.def"
#undef X
	};
	class DecodedInstruction {
		enum class Fields {
#define X(en, u0, u1, u2, u3, u4) en ,
#include "iris32_instruction.def"
#undef X
			Count,
		};
		public:
			DecodedInstruction(word rinst);
#define X(field, mask, shift, type, isreg, unused) \
			type get ## field (); \
			void set ## field (type value);
#include "iris32_instruction.def"
#undef X
			word encodeInstruction();
		private:
#define X(u0, u1, u2, type, u3, fieldName) type fieldName; 
#include "iris32_instruction.def"
#undef X
			word raw;
	};
	/// Represents the execution state of a thread of execution
	struct ExecState {
		bool advanceIp = true;
		word gpr[ArchitectureConstants::RegisterCount] = { 0 };
	};

	class Core : public iris::Core {
		public:
			Core(word memorySize, ExecState&& t0, ExecState&& t1);
			~Core();
			virtual void initialize();
			virtual void installprogram(std::istream& stream);
			virtual void shutdown();
			virtual void dump(std::ostream& stream);
			virtual void run();
		private:
			void write(word address, word value);
			word read(word address);
			void execBody(ExecState& thread);
			void decode(ExecState& curr);
			void dispatch(ExecState& curr);
#define X(_, func) void func (ExecState& thread, DecodedInstruction& inst); 
#include "iris32_groups.def"
#undef X
		private:
			word memorySize;
			word* memory;
			ExecState thread0,
					  thread1;
			bool execute = true;
	};

} // end namespace iris32
#endif // end _TARGET_IRIS32_IRIS_H
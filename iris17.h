#ifndef _TARGET_IRIS17_IRIS_H
#define _TARGET_IRIS17_IRIS_H
#include "iris_base.h"
#include "Core.h"
#include <cstdint>
#include <vector>
#include <memory>


namespace iris17 {
	using dword = int64_t;
    using word = int32_t;
    using hword = int16_t;
    constexpr word encodeWord(byte, byte, byte, byte) noexcept;
    enum ArchitectureConstants  {
        RegisterCount = 256,
        AddressMax = 268435456 /* bytes */ / sizeof(word), // words
        InstructionPointerIndex = RegisterCount - 1,
        LinkRegisterIndex = RegisterCount - 2,
        StackPointerIndex = RegisterCount - 3,
        ConditionRegisterIndex = RegisterCount - 4,
        ThreadIndex = RegisterCount - 5,

        GroupMask = 0b00000111,
        RestMask = ~GroupMask,
        MaxInstructionsPerGroup = RestMask >> 3,
        MaxGroups = 8,
    };

#include "iris17_defines.h"

class DecodedInstruction {
    public:
        DecodedInstruction(word rinst);
        word getRawValue() const { return raw; }
        inline byte getDestination() const noexcept { return decodeDestination(raw); }
        inline byte getSource0() const noexcept { return decodeSource0(raw); }
        inline byte getSource1() const noexcept { return decodeSource1(raw); }
        inline hword getImmediate() const noexcept { return decodeImmediate(raw); }
        inline byte getGroup() const noexcept { return decodeGroup(raw); }
        inline byte getOperation() const noexcept { return decodeOperation(raw); }
        inline byte getControl() const noexcept { return decodeControl(raw); }
    private:
        word raw;
};
/// Represents the execution state of a thread of execution
struct ExecState {
    bool advanceIp = true;
    word gpr[ArchitectureConstants::RegisterCount] = { 0 };
};

class Core : public iris::Core {
    public:
        Core(word memorySize, byte numThreads);
        ~Core();
        virtual void initialize();
        virtual void installprogram(std::istream& stream);
        virtual void shutdown();
        virtual void dump(std::ostream& stream);
        virtual void run();
        virtual void link(std::istream& input);
        void write(word address, word value);
        word read(word address);
    private:
        void execBody();
        void decode();
        void dispatch();
        void systemCall(DecodedInstruction& inst);
    private:
        void compare(DecodedInstruction&& inst);
        void jump(DecodedInstruction&& inst);
        void move(DecodedInstruction&& inst);
        void arithmetic(DecodedInstruction&& inst);
        void misc(DecodedInstruction&& inst);
    private:
        word memorySize;
        std::unique_ptr<word> memory;
        std::shared_ptr<ExecState> thread;
        std::vector<std::shared_ptr<ExecState>> threads;
        bool execute = true;
};

Core* newCore() noexcept;
void assemble(FILE* input, std::ostream* output);
} // end namespace iris17
#undef DefOp
#endif // end _TARGET_IRIS17_IRIS_H

/*
 * syn
 * Copyright (c) 2013-2017, Joshua Scoggins and Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _TARGET_CISC0_IRIS_H
#define _TARGET_CISC0_IRIS_H
#include "Base.h"
#include "ExecutionUnits.h"
#include "Core.h"
#include "Problem.h"
#include <cstdint>
#include <sstream>
#include <memory>
#include <vector>
#include <tuple>
#include "IODevice.h"

namespace cisc0 {
	using HWord = uint8_t;
	using Word = uint16_t;
	using DWord = uint32_t;
	using RawInstruction = Word; // this is more of a packet!
	using immediate = HWord;
	using RegisterValue = DWord;
    using Address = DWord;
	constexpr Word encodeWord (byte a, byte b) noexcept;
	constexpr RegisterValue encodeRegisterValue(byte a, byte b, byte c, byte d) noexcept;
	void decodeWord(Word value, byte* storage) noexcept;
	void decodeRegisterValue(RegisterValue value, byte* storage) noexcept;
	constexpr Word decodeUpperHalf(RegisterValue value) noexcept;
	constexpr Word decodeLowerHalf(RegisterValue value) noexcept;
	constexpr RegisterValue encodeUpperHalf(RegisterValue value, Word upperHalf) noexcept;
	constexpr RegisterValue encodeLowerHalf(RegisterValue value, Word lowerHalf) noexcept;
	constexpr RegisterValue encodeRegisterValue(Word upper, Word lower) noexcept;

	enum ArchitectureConstants  {
		RegisterCount = 16,
		SegmentCount = 256,
		AddressMax = 65536 * SegmentCount,
		MaxInstructionCount = 16,
		MaxSystemCalls = 0x1000,
		Bitmask = 0b1111,
		// unlike iris16 and iris32, there is a limited set of registers with
		// a majority of them marked for explicit usage, instructions
		// themselves are still 16 bits wide but 32bits are extracted per
		// packet.
		R15 = RegisterCount - 1,
		R14 = RegisterCount - 2,
		R13 = RegisterCount - 3,
		R12 = RegisterCount - 4,
		R11 = RegisterCount - 5,
		R10 = RegisterCount - 6,
		R9  = RegisterCount - 7,
		R8  = RegisterCount - 8,
		R7  = RegisterCount - 9,
		R6  = RegisterCount - 10,
		R5  = RegisterCount - 11,
		R4  = RegisterCount - 12,
		R3  = RegisterCount - 13,
		R2  = RegisterCount - 14,
		R1  = RegisterCount - 15,
		R0  = RegisterCount - 16,
		InstructionPointer = R15,
		StackPointer = R14,
		ConditionRegister = R13,
		AddressRegister = R12,
		ValueRegister = R11,
		MaskRegister = R10,
		ShiftRegister = R9,
		FieldRegister = R9,
	};
} // end namespace cisc0

#include "cisc0_defines.h"

namespace cisc0 {
	class DecodedInstruction {
		public:
			DecodedInstruction(RawInstruction input) noexcept : _rawValue(input) { }
			DecodedInstruction(const DecodedInstruction&) = delete;
            virtual ~DecodedInstruction() { }
			RawInstruction getRawValue() const noexcept { return _rawValue; }
            using BranchFlags = std::tuple<bool, bool, bool>;
            inline byte getUpper() const noexcept { return decodeUpper(_rawValue); }
            inline Operation getControl() const noexcept { return decodeControl(_rawValue); }
            inline byte getSetDestination() const noexcept { return decodeSetDestination(_rawValue); }
            inline byte getMemoryRegister() const noexcept { return decodeMemoryRegister(_rawValue); }
            inline byte getBranchIndirectDestination() const noexcept { return decodeBranchIndirectDestination(_rawValue); }
            inline bool shouldShiftLeft() const noexcept { return decodeShiftFlagLeft(_rawValue); }
            inline bool isIndirectOperation() const noexcept { return decodeMemoryFlagIndirect(_rawValue); }
            inline byte getMemoryOffset() const noexcept { return decodeMemoryOffset(_rawValue); }
            BranchFlags getOtherBranchFlags() const noexcept;
            inline EncodingOperation getEncodingOperation() const noexcept { return decodeComplexClassEncoding_Type(_rawValue); }

            template<int index>
            inline byte getShiftRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal shift register index!");
                if (index == 0) {
                    return decodeShiftRegister0(_rawValue);
                } else {
                    return decodeShiftRegister1(_rawValue);
                }
            }
            template<int index>
            inline byte getSystemArg() const noexcept {
                static_assert(index >= 0 && index < 3, "Illegal system arg index!");
                if (index == 0) {
                    return decodeSystemArg0(_rawValue);
                } else if (index == 1) {
                    return decodeSystemArg1(_rawValue);
                } else {
                    return decodeSystemArg2(_rawValue);
                }
            }
            template<int index>
            inline byte getMoveRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal move register index!");
                if (index == 0) {
                    return decodeMoveRegister0(_rawValue);
                } else {
                    return decodeMoveRegister1(_rawValue);
                }
            }
            template<int index>
            inline byte getSwapRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal swap register index!");
                if (index == 0) {
                    return decodeSwapDestination(_rawValue);
                } else {
                    return decodeSwapSource(_rawValue);
                }
            }
            template<int index>
            inline byte getCompareRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal compare register index!");
                if (index == 0) {
                    return decodeCompareRegister0(_rawValue);
                } else {
                    return decodeCompareRegister1(_rawValue);
                }
            }
            template<int index>
            inline byte getArithmeticRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal arithmetic register index!");
                if (index == 0) {
                    return decodeArithmeticDestination(_rawValue);
                } else {
                    return decodeArithmeticSource(_rawValue);
                }
            }
            static constexpr bool hasBitmask(Operation op) noexcept {
                return op == Operation::Set ||
                       op == Operation::Memory ||
                       op == Operation::Move ||
                       op == Operation::Logical;
            }
            template<Operation op>
            inline byte getBitmask() const noexcept {
                static_assert(hasBitmask(op), "provided operation does not use a bitmask!");
                switch(op) {
                    case Operation::Set:
                        return decodeSetBitmask(_rawValue);
                    case Operation::Memory:
                        return decodeMemoryFlagBitmask(_rawValue);
                    case Operation::Move:
                        return decodeMoveBitmask(_rawValue);
                    case Operation::Logical:
                        return decodeLogicalFlagImmediateMask(_rawValue);
                    default:
                        return 0;
                }
            }
            static constexpr bool hasImmediateFlag(Operation op) noexcept {
                return op == Operation::Shift ||
                       op == Operation::Arithmetic ||
                       op == Operation::Logical ||
                       op == Operation::Branch ||
                       op == Operation::Compare;
            }
            template<Operation op>
            inline bool getImmediateFlag() const noexcept {
                static_assert(hasImmediateFlag(op), "provided operation does not have an immediate flag!");
                switch(op) {
                    case Operation::Shift:
                        return decodeShiftFlagImmediate(_rawValue);
                    case Operation::Arithmetic:
                        return decodeArithmeticFlagImmediate(_rawValue);
                    case Operation::Logical:
                        return decodeLogicalFlagImmediate(_rawValue);
                    case Operation::Branch:
                        return decodeBranchFlagIsImmediate(_rawValue);
                    case Operation::Compare:
                        return decodeCompareImmediateFlag(_rawValue);
                    default:
                        return false;
                }
            }
            static constexpr bool hasImmediateValue(Operation op) noexcept {
                return op == Operation::Shift ||
                      op == Operation::Arithmetic;
            }
            template<Operation op>
            inline byte getImmediate() const noexcept {
                static_assert(hasImmediateValue(op), "provided operation cannot contain an immediate value!");
                switch(op) {
                    case Operation::Shift:
                        return decodeShiftImmediate(_rawValue);
                    case Operation::Arithmetic:
                        return decodeArithmeticImmediate(_rawValue);
                    default:
                        return 0;
                }
            }
            template<int index>
            inline byte getLogicalRegister() const noexcept {
                static_assert(index >= 0 && index < 2, "Illegal logical register index!");
                if (index == 0) {
                    if (getImmediateFlag<Operation::Logical>()) {
                        return decodeLogicalImmediateDestination(_rawValue);
                    } else {
                        return decodeLogicalRegister0(_rawValue);
                    }
                } else {
                    if (getImmediateFlag<Operation::Logical>()) {
                        throw syn::Problem("There is no second register argument for an immediate logical operation!");
                    } else {
                        return decodeLogicalRegister1(_rawValue);
                    }
                }
            }
            template<bool path>
            inline byte getBranchIfPathRegister() const noexcept {
                if (path) {
                    return decodeBranchIfOnTrue(_rawValue);
                } else {
                    return decodeBranchIfOnFalse(_rawValue);
                }
            }
            static constexpr bool hasSubtype(Operation op) noexcept {
                return op == Operation::Compare ||
                    op == Operation::Memory ||
                    op == Operation::Arithmetic ||
                    op == Operation::Complex ||
                    op == Operation::Logical;

            }
            template<Operation op>
            struct SubtypeConversion { };

            template<Operation op>
            inline typename SubtypeConversion<op>::ResultantType getSubtype() const noexcept {
                static_assert(hasSubtype(op), "There is no subtype for the given operation!");
                using CurrentType = typename SubtypeConversion<op>::ResultantType;
                switch(op) {
                    case Operation::Compare:
                        return static_cast<CurrentType>(decodeCompareType(_rawValue));
                    case Operation::Arithmetic:
                        return static_cast<CurrentType>(decodeArithmeticFlagType(_rawValue));
                    case Operation::Memory:
                        return static_cast<CurrentType>(decodeCompareType(_rawValue));
                    case Operation::Complex:
                        return static_cast<CurrentType>(decodeComplexSubClass(_rawValue));
                    case Operation::Logical:
                        return static_cast<CurrentType>(decodeLogicalFlagType(_rawValue));
                    default:
                        return static_cast<CurrentType>(0);
                }
            }

		private:
			RawInstruction _rawValue;
	};

	inline constexpr Word lowerMask(byte bitmask) noexcept {
		return syn::encodeUint16LE(syn::expandBit(syn::getBit<byte, 0>(bitmask)),
									syn::expandBit(syn::getBit<byte, 1>(bitmask)));
	}
	inline constexpr Word upperMask(byte bitmask) noexcept {
		return syn::encodeUint16LE(syn::expandBit(syn::getBit<byte, 2>(bitmask)),
									syn::expandBit(syn::getBit<byte, 3>(bitmask)));
	}

	inline constexpr RegisterValue mask(byte bitmask) noexcept {
		return syn::encodeUint32LE(lowerMask(bitmask), upperMask(bitmask));
	}

	inline constexpr bool readLower(byte bitmask) noexcept {
		return lowerMask(bitmask) != 0;
	}

	inline constexpr bool readUpper(byte bitmask) noexcept {
		return upperMask(bitmask) != 0;
	}
	constexpr auto bitmask32 =   mask(ArchitectureConstants::Bitmask);
	constexpr auto bitmask24 =   mask(0b0111);
	constexpr auto upper16Mask = mask(0b1100);
	constexpr auto lower16Mask = mask(0b0011);

	int instructionSizeFromImmediateMask(byte bitmask) noexcept;

	class Core : public syn::Core {
		public:
            using ALU = syn::ALU<RegisterValue>;
            using CompareUnit = syn::Comparator<RegisterValue>;
            using RegisterFile = syn::FixedSizeLoadStoreUnit<RegisterValue, byte, ArchitectureConstants::RegisterCount>;
            using MemorySpace = syn::FixedSizeLoadStoreUnit<Word, Address, ArchitectureConstants::AddressMax>;
            using RandomNumberGenerator = syn::CaptiveAddressableIODevice<syn::RandomDevice<RegisterValue, Address>>;
			using SystemFunction = std::function<void(Core*, DecodedInstruction&&)>;
			// These are built in addresses
			enum DefaultHandlers {
				Terminate,
				GetC,
				PutC,
				SeedRandom,
				NextRandom,
				SkipRandom,
				SecondaryStorage0_Read,
				SecondaryStorage0_Write,
				SecondaryStorage1_Read,
				SecondaryStorage1_Write,
				Count,
			};
			static_assert(static_cast<Word>(DefaultHandlers::Count) <= static_cast<Word>(ArchitectureConstants::MaxSystemCalls), "Too many handlers defined!");
		public:
			Core() noexcept;
			virtual ~Core() noexcept;
			virtual void initialize() override;
			virtual void installprogram(std::istream& stream) override;
			virtual void shutdown() override;
			virtual void dump(std::ostream& stream) override;
			virtual void link(std::istream& stream) override;
			void installSystemHandler(Word index, SystemFunction fn);
			virtual bool cycle() override;
			bool shouldExecute() const { return execute; }
		private:
			void pushWord(Word value);
            void pushWord(Word value, RegisterValue& ptr);
			void pushDword(DWord value);
            void pushDword(DWord value, RegisterValue& ptr);
			Word popWord();
			Word popWord(RegisterValue& ptr);
			static void defaultSystemHandler(Core* core, DecodedInstruction&& inst);
			static void terminate(Core* core, DecodedInstruction&& inst);
			static void getc(Core* core, DecodedInstruction&& inst);
			static void putc(Core* core, DecodedInstruction&& inst);
			static void seedRandom(Core* core, DecodedInstruction&& inst);
			static void nextRandom(Core* core, DecodedInstruction&& inst);
			static void skipRandom(Core* core, DecodedInstruction&& inst);
			SystemFunction getSystemHandler(byte index);
			void dispatch(DecodedInstruction&& inst);
			template<byte rindex>
				inline RegisterValue& registerValue() noexcept {
					static_assert(rindex < ArchitectureConstants::RegisterCount, "Not a legal register index!");
					return gpr[rindex];
				}
            template<bool readNext>
            inline Word tryReadNext() noexcept {
                if (readNext) {
                    incrementInstructionPointer();
                    return getCurrentCodeWord();
                } else {
                    return 0;
                }
            }
            Word tryReadNext(bool readNext) noexcept;
			RegisterValue retrieveImmediate(byte bitmask) noexcept;

			RegisterValue& registerValue(byte index);
			inline RegisterValue& getInstructionPointer() noexcept     { return registerValue<ArchitectureConstants::InstructionPointer>(); }
			inline RegisterValue& getStackPointer() noexcept           { return registerValue<ArchitectureConstants::StackPointer>(); }
			inline RegisterValue& getConditionRegister() noexcept      { return registerValue<ArchitectureConstants::ConditionRegister>(); }
			inline RegisterValue& getAddressRegister() noexcept        { return registerValue<ArchitectureConstants::AddressRegister>(); }
			inline RegisterValue& getValueRegister() noexcept          { return registerValue<ArchitectureConstants::ValueRegister>(); }
			inline RegisterValue& getMaskRegister() noexcept           { return registerValue<ArchitectureConstants::MaskRegister>(); }

			inline RegisterValue getShiftRegister() noexcept           { return 0b11111 & registerValue<ArchitectureConstants::ShiftRegister>(); }
			inline RegisterValue getFieldRegister() noexcept           { return 0b11111 & registerValue<ArchitectureConstants::FieldRegister>(); }

			void incrementInstructionPointer() noexcept;
			void incrementStackPointer() noexcept;
			void decrementStackPointer() noexcept;
			void decrementStackPointer(RegisterValue& ptr) noexcept;
			void incrementStackPointer(RegisterValue& ptr) noexcept;
			void incrementAddress(RegisterValue& ptr) noexcept;
			void decrementAddress(RegisterValue& ptr) noexcept;
			Word getCurrentCodeWord() noexcept;
			void storeWord(RegisterValue address, Word value);
			Word loadWord(RegisterValue address);
			RegisterValue loadRegisterValue(RegisterValue address);
			void storeRegisterValue(RegisterValue address, RegisterValue value);
		private:
			void complexOperation(DecodedInstruction&& inst);
			void encodingOperation(DecodedInstruction&& inst);
			void performEncodeOp(DecodedInstruction&& inst);
        private:
            void compareOperation(DecodedInstruction&& inst);
            void systemCallOperation(DecodedInstruction&& inst);
            void branchOperation(DecodedInstruction&& inst);
            void memoryOperation(DecodedInstruction&& inst);
            void logicalOperation(DecodedInstruction&& inst);
            void arithmeticOperation(DecodedInstruction&& inst);
            void shiftOperation(DecodedInstruction&& inst);
		private:
			bool execute = true,
				 advanceIp = true;
			RegisterFile gpr;
			ALU _alu;
			ALU _shifter;
			ALU _logicalOps;
			CompareUnit _compare;
			syn::BooleanCombineUnit _bCombine;
			MemorySpace memory;
			SystemFunction systemHandlers[ArchitectureConstants::MaxSystemCalls] =  { 0 };
			RandomNumberGenerator _rng;
	};


	struct InstructionEncoder {
		int currentLine;
		RegisterValue address;
		Operation type;
		bool immediate;
		bool shiftLeft;
		bool isIf;
		bool isCall;
		bool isConditional;
		bool indirect;
		bool readNextWord;
		byte bitmask;
		byte arg0;
		byte arg1;
		byte arg2;
		bool isLabel;
		std::string labelValue;
		byte subType;
		RegisterValue fullImmediate;
		using Encoding = std::tuple<int, Word, Word, Word>;
		int numWords() const;
		Encoding encode() const;
		void clear();
		private:
            Encoding encodeMemory() const;
            Encoding encodeArithmetic() const;
            Encoding encodeShift() const;
            Encoding encodeLogical() const;
            Encoding encodeCompare() const;
            Encoding encodeBranch() const;
            Encoding encodeSystemCall() const;
            Encoding encodeMove() const;
            Encoding encodeSet() const;
            Encoding encodeSwap() const;
            Encoding encodeComplex() const;
	};
	Core* newCore() noexcept;
	void assemble(const std::string& iName, FILE* input, std::ostream* output);
    template<> struct DecodedInstruction::SubtypeConversion<Operation::Compare> { using ResultantType = CompareStyle; };
    template<> struct DecodedInstruction::SubtypeConversion<Operation::Arithmetic> { using ResultantType = ArithmeticOps; };
    template<> struct DecodedInstruction::SubtypeConversion<Operation::Memory> { using ResultantType = MemoryOperation; };
    template<> struct DecodedInstruction::SubtypeConversion<Operation::Complex> { using ResultantType = ComplexSubTypes; };
    template<> struct DecodedInstruction::SubtypeConversion<Operation::Logical> { using ResultantType = LogicalOps; };
} // end namespace cisc0

#endif // end _TARGET_CISC0_IRIS_H

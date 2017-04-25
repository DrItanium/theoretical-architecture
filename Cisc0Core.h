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
#include <sstream>
#include <memory>
#include <vector>
#include <tuple>

#include "Base.h"
#include "ExecutionUnits.h"
#include "Core.h"
#include "Problem.h"
#include "IODevice.h"
#include "IOController.h"
#include "Cisc0CoreConstants.h"
#include "Cisc0CoreDecodedInstruction.h"

extern "C" {
	#include "clips.h"
}

namespace cisc0 {
	class Core : public syn::Core {
		public:
			using IOBus = syn::CLIPSIOController<Word, CLIPSInteger>;
            using RegisterFile = syn::FixedSizeLoadStoreUnit<RegisterValue, byte, ArchitectureConstants::RegisterCount>;
        public:
			static Core* make() noexcept;
		public:
			Core() noexcept;
			virtual ~Core() noexcept;
			virtual void initialize() override;
			virtual void shutdown() override;
			virtual bool cycle() override;
			bool shouldExecute() const noexcept { return execute; }
			bool handleOperation(void* env, CLIPSValue* ret);
		private:
			void pushWord(Word value);
            void pushWord(Word value, RegisterValue& ptr);
			void pushDword(DWord value);
            void pushDword(DWord value, RegisterValue& ptr);
			Word popWord();
			Word popWord(RegisterValue& ptr);
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
			RegisterValue& getInstructionPointer() noexcept     { return registerValue<ArchitectureConstants::InstructionPointer>(); }
			RegisterValue& getStackPointer() noexcept           { return registerValue<ArchitectureConstants::StackPointer>(); }
			RegisterValue& getConditionRegister() noexcept      { return registerValue<ArchitectureConstants::ConditionRegister>(); }
			RegisterValue& getAddressRegister() noexcept        { return registerValue<ArchitectureConstants::AddressRegister>(); }
			RegisterValue& getValueRegister() noexcept          { return registerValue<ArchitectureConstants::ValueRegister>(); }
			RegisterValue& getMaskRegister() noexcept           { return registerValue<ArchitectureConstants::MaskRegister>(); }

			RegisterValue getShiftRegister() noexcept           { return 0b11111 & registerValue<ArchitectureConstants::ShiftRegister>(); }
			RegisterValue getFieldRegister() noexcept           { return 0b11111 & registerValue<ArchitectureConstants::FieldRegister>(); }

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
			IOBus _bus;
	};


} // end namespace cisc0

#endif // end _TARGET_CISC0_IRIS_H

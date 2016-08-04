#include "iris17.h"
#include <functional>
#include <sstream>
#include "Problem.h"

namespace iris17 {
/*
 * Iris17 is a variable length encoding 16 bit architecture.
 * It has a 24 bit memory space across 256 16-bit sections. The variable length
 * encoding comes from different register choices. The reserved registers are
 * used to compress the encoding.
 */
	Core* newCore() {
		return new Core();
	}
	RegisterValue encodeRegisterValue(byte a, byte b, byte c, byte d) {
		return iris::encodeUint32LE(a, b, c, d);
	}
	word encodeWord(byte a, byte b) {
		return iris::encodeUint16LE(a, b);
	}
	void decodeWord(word value, byte* storage) {
		return iris::decodeUint32LE(value, storage);
	}
	void decodeWord(RegisterValue value, byte* storage) {
		return iris::decodeInt32LE(value, storage);
	}

	DecodedInstruction::DecodedInstruction(raw_instruction input) : _rawValue(input) { }

	raw_instruction DecodedInstruction::getRawValue() const {
		return _rawValue;
	}

#define X(title, mask, shift, type, is_register, post) \
		type DecodedInstruction:: get ## title () const { \
			return iris::decodeBits<raw_instruction, type, mask, shift>(_rawValue); \
		}
#include "def/iris17/instruction.def"
#undef X


	Core::Core() : memory(new word[ArchitectureConstants::AddressMax]) { }
	Core::~Core() {
		delete [] memory;
	}

	void Core::initialize() { }

	void Core::shutdown() { }

	template<typename T, int count>
	void populateContents(T* contents, std::istream& stream, std::function<T(byte*)> encode) {
		static char buf[sizeof(word)] = { 0 };
		for(int i = 0; i < count; ++i) {
			stream.read(buf, sizeof(word));
			contents[i] = encode((byte*)buf);
		}
	}
	void Core::installprogram(std::istream& stream) {

		populateContents<RegisterValue, ArchitectureConstants::RegisterCount>(gpr, stream, [](byte* buf) { return iris::encodeUint32LE(buf); });
		populateContents<word, ArchitectureConstants::AddressMax>(memory, stream, [](byte* buf) { return iris::encodeUint16LE(buf); });
	}

	template<typename T, int count>
	void dumpContents(T* contents, std::ostream& stream, std::function<void(T value, byte* buf)> decompose) {
		static byte buf[sizeof(T)];
		for (int i = 0; i < count; ++i) {
			decompose(contents[i], (byte*)buf);
			stream.write((char*)buf, sizeof(T));
		}
	}

	void Core::dump(std::ostream& stream) {
		// save the registers
		dumpContents<RegisterValue, ArchitectureConstants::RegisterCount>(gpr, stream, iris::decodeUint32LE);
		dumpContents<word, ArchitectureConstants::AddressMax>(memory, stream, iris::decodeUint16LE);
	}
	void Core::run() {
		while(execute) {
			DecodedInstruction di(getCurrentCodeWord());
			dispatch(std::move(di));
			if (advanceIp) {
				++getInstructionPointer();
			} else {
				// just re-enable it
				advanceIp = true;
			}
		}
	}

#define DefOp(title) \
	template<> \
	void Core::operation<Operation:: title>(DecodedInstruction&& current) 
	
	DefOp(Nop) { 
	}

#define X(title, operation) \
	DefOp(title) { \
		registerValue(current.getDestination()) = registerValue(current.getDestination()) operation (current.getArithmeticFlagImmediate() ? current.getArithmeticImmediate() : registerValue(current.getSrc0())); \
	}
// for the cases where we have an immediate form
#define Y(title, operation) \
	DefOp(title) { \
		RegisterValue src1 = (current.getArithmeticFlagImmediate() ? current.getArithmeticImmediate() : registerValue(current.getSrc0())); \
		if (src1 == 0) { \
			throw iris::Problem("Denominator is zero!"); \
		} else { \
			registerValue(current.getDestination()) = registerValue(current.getDestination()) operation src1; \
		} \
	}
#include "def/iris17/arithmetic_ops.def"
#undef X
#undef Y

	DefOp(BinaryNot) {
		registerValue(current.getDestination()) = ~registerValue(current.getDestination());
	}

	DefOp(Increment) {
		++registerValue(current.getDestination());
	}
	
	DefOp(Decrement) {
		--registerValue(current.getDestination());
	}

	DefOp(Double) {
		registerValue(current.getDestination()) *= 2;
	}

	DefOp(Halve) {
		registerValue(current.getDestination()) /= 2;
	}

	DefOp(Move)  {
		registerValue(current.getDestination()) = registerValue(current.getSrc0());
	}

	DefOp(Swap) {
		RegisterValue tmp = registerValue(current.getDestination());
		registerValue(current.getDestination()) = registerValue(current.getSrc0());
		registerValue(current.getSrc0()) = tmp;
	}

	template<byte bitmask> 
	struct SetBitmaskToWordMask {
		static constexpr bool decomposedBits[] = {
			(bitmask & 0b0001),
			((bitmask & 0b0010) >> 1),
			((bitmask & 0b0100) >> 2),
			((bitmask & 0b1000) >> 3),
		};
		static constexpr byte determineMaskValue(bool value) { return value ? 0xFF : 0x00; }
		static constexpr RegisterValue mask = (determineMaskValue(decomposedBits[3]) << 24) |
				(determineMaskValue(decomposedBits[2]) << 16) | 
				(determineMaskValue(decomposedBits[1]) << 8) | 
				(determineMaskValue(decomposedBits[0]));
		static constexpr word lowerMask = (determineMaskValue(decomposedBits[1]) << 8) | (determineMaskValue(decomposedBits[0]));
		static constexpr word upperMask = (determineMaskValue(decomposedBits[3]) << 8) | (determineMaskValue(decomposedBits[2]));
		static constexpr bool readLower = decomposedBits[1] || decomposedBits[0];
		static constexpr bool readUpper = decomposedBits[2] || decomposedBits[3];
	};
	template<byte bitmask>
	constexpr RegisterValue mask() { return SetBitmaskToWordMask<bitmask>::mask; }
	template<byte bitmask>
	constexpr word lowerMask() { return SetBitmaskToWordMask<bitmask>::lowerMask; }
	template<byte bitmask>
	constexpr word upperMask() { return SetBitmaskToWordMask<bitmask>::upperMask; }
	template<byte bitmask>
	constexpr bool readLower() { return SetBitmaskToWordMask<bitmask>::readLower; }
	template<byte bitmask>
	constexpr bool readUpper() { return SetBitmaskToWordMask<bitmask>::readUpper; }


    DefOp(Set) {
		auto bitmask = registerValue(current.getSrc0());
		switch (bitmask) {
#define X(value) \
			case value: \
			{ \
				RegisterValue lower = 0; \
				RegisterValue upper = 0; \
				if ( readLower<value>() ) { \
					++getInstructionPointer(); \
					lower = getCurrentCodeWord(); \
				} \
				if (readUpper<value>() ) { \
					++getInstructionPointer(); \
					upper = RegisterValue(getCurrentCodeWord()) << 16; \
				} \
				registerValue(current.getDestination()) = mask<value>() & (lower | upper); \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("unknown mask!");
		}
    }
	void Core::storeWord(RegisterValue address, word value) {
		if (address > ArchitectureConstants::AddressMax) {
			throw iris::Problem("Attempted to write outside of memory!");
		} else {
			memory[address] = value;
		}
	}
	word Core::loadWord(RegisterValue address) {
		if (address > ArchitectureConstants::AddressMax) {
			throw iris::Problem("Attempted to read from outside of memory!");
		} else {
			return memory[address];
		}
	}
	RegisterValue Core::loadRegisterValue(RegisterValue address) {
		return iris::encodeBits<RegisterValue, word, mask<0b1111>(), 16>(RegisterValue(loadWord(address)), loadWord(address + 1));
	}
	void Core::storeRegisterValue(RegisterValue address, RegisterValue value) {
		storeWord(address, iris::decodeBits<RegisterValue, word, mask<0b0011>(), 0>(value));
		storeWord(address + 1, iris::decodeBits<RegisterValue, word, mask<0b1100>(), 16>(value));
	}
	DefOp(Load) {
		// use the destination field of the instruction to denote offset, thus we need
		// to use the Address and Value registers
		auto offset = current.getDestination();
		RegisterValue address = getAddressRegister() + offset;
		// use the src0 field of the instruction to denote the bitmask
		auto bitmask = current.getSrc0();
		switch (bitmask) {
#define X(value) \
			case value: \
			{ \
				RegisterValue lower = 0; \
				RegisterValue upper = 0; \
				if (readLower<value>()) { \
					lower = loadWord(address); \
				} \
				if (readUpper<value>()) { \
					upper = RegisterValue(loadWord(address + 1)) << 16; \
				} \
				getValueRegister() = (mask<value>() & (lower | upper)); \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("illegal bitmask");
		}
	}

	DefOp(LoadMerge) {
		// use the destination field of the instruction to denote offset, thus we need
		// to use the Address and Value registers
		auto offset = current.getDestination();
		RegisterValue address = getAddressRegister() + offset;
		// use the src0 field of the instruction to denote the bitmask
		auto bitmask = current.getSrc0();
		switch (bitmask) {
			// 0b1101 implies that we have to leave 0x0000FF00 around in the
			// value register since it isn't necessary
#define X(value) \
			case value: \
			{ \
				RegisterValue lower = 0; \
				RegisterValue upper = 0; \
				if (readLower<value>()) { \
					lower = loadWord(address); \
				} \
				if (readUpper<value>()) { \
					upper = RegisterValue(loadWord(address + 1)) << 16; \
				} \
				getValueRegister() = (mask<value>() & (lower | upper)) | (getValueRegister() & ~mask<value>()); \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("illegal bitmask");
		}
	}

	DefOp(Store) {
		auto offset = current.getDestination();
		auto bitmask = current.getSrc0();
		RegisterValue address = getAddressRegister() + offset;
		switch(bitmask) {
			// 0b1101 implies that we have to leave 0x0000FF00 around in the
			// value register since it isn't necessary
#define X(value) \
			case value: \
			{ \
				RegisterValue lower = 0; \
				RegisterValue upper = 0; \
				if (readLower<value>()) { \
					lower = lowerMask<value>() & iris::decodeBits<RegisterValue, word, mask<0b0011>(), 0>(getValueRegister()); \
					auto loader = loadWord(address) & ~lowerMask<value>(); \
					storeWord(address, lower | loader); \
				} \
				if (readUpper<value>()) { \
					upper = upperMask<value>() & iris::decodeBits<RegisterValue, word, mask<0b1100>(), 16>(getValueRegister()); \
					auto loader = loadWord(address) & ~upperMask<value>(); \
					storeWord(address + 1, upper | loader); \
				} \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("illegal bitmask");
		}
	}

	DefOp(Push) {
		constexpr RegisterValue bitmask24 = mask<0b0111>();
		constexpr RegisterValue upper16 = mask<0b1100>();
		constexpr RegisterValue lower16 = mask<0b0011>();
		auto stackPointer = getStackPointer();
		auto bitmask = current.getSrc0();
		auto pushToStack = registerValue(current.getSrc1());
		switch (bitmask) {
#define X(value) \
			case value: \
			{ \
				if (readUpper<value>()) { \
					--stackPointer; \
					stackPointer &= bitmask24; \
					RegisterValue upper = upperMask<value>() & iris::decodeBits<RegisterValue, word, upper16, 16>(pushToStack); \
					storeWord(stackPointer, upper); \
				} \
				if (readLower<value>()) { \
					--stackPointer; \
					stackPointer &= bitmask24; \
					RegisterValue lower = lowerMask<value>() & iris::decodeBits<RegisterValue, word, lower16, 0>(pushToStack); \
					storeWord(stackPointer, lower); \
				} \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("illegal bitmask");
		}
	}

	DefOp(Pop) {
		constexpr RegisterValue bitmask24 = mask<0b0111>();
		constexpr RegisterValue upper16 = mask<0b1100>();
		constexpr RegisterValue lower16 = mask<0b0011>();
		auto stackPointer = getStackPointer();
		auto bitmask = current.getSrc0();
		switch (bitmask) {
			// pop the entries off of the stack and store it in the register
#define X(value) \
			case value: \
			{ \
				RegisterValue lower = 0; \
				RegisterValue upper = 0; \
				if (readLower<value>()) { \
					auto val = loadWord(stackPointer); \
					++stackPointer; \
					stackPointer &= bitmask24; \
					lower = lowerMask<value>() & val; \
				} \
				if (readUpper<value>()) { \
					auto val = loadWord(stackPointer); \
					++stackPointer; \
					stackPointer &= bitmask24; \
					upper = upperMask<value>() & val; \
				} \
				registerValue(current.getSrc1()) = iris::encodeBits<word, RegisterValue, upper16, 16>(iris::encodeBits<word, RegisterValue, lower16, 0>(RegisterValue(0), lower), upper); \
				break; \
			}
#include "def/iris17/bitmask4bit.def"
#undef X
			default:
				throw iris::Problem("illegal bitmask");
		}
	}
	template<bool indirect, bool conditional>
	struct BranchFlags_BoolsToByte {
		static constexpr byte flag = (static_cast<byte>(conditional) << 1) | static_cast<byte>(indirect);
	};
	template<byte value>
	struct BranchFlags_ByteToBools {
		static constexpr bool isImmediate = static_cast<bool>(value & 0b11);
		static constexpr bool isConditional = static_cast<bool>((value & 0b11) >> 1);
	};
	template<byte flags>
	bool branchSpecificOperation(RegisterValue& ip, RegisterValue cond, std::function<RegisterValue()> getUpper16, std::function<RegisterValue(byte)> registerValue, DecodedInstruction&& current) {
		constexpr RegisterValue bitmask24 = mask<0b0111>();
		bool advanceIp = true;
		if (BranchFlags_ByteToBools<flags>::isImmediate) {
			++ip;
			if (BranchFlags_ByteToBools<flags>::isConditional) {
				if (cond != 0) {
					advanceIp = false;
					auto bottom = current.getUpper();
					auto upper = getUpper16() << 8;
					ip = bitmask24 & (upper | bottom);
				}
			} else {
				advanceIp = false;
				auto bottom = RegisterValue(current.getUpper());
				auto upper = getUpper16() << 8;
				ip = bitmask24 & (upper | bottom);
			}
		}  else {
			if (BranchFlags_ByteToBools<flags>::isConditional) {
				if (cond != 0) {
					advanceIp = false;
					auto target = registerValue(current.getBranchIndirectDestination());
					ip = bitmask24 & target;
				}
			} else {
				advanceIp = false;
				auto target = registerValue(current.getBranchIndirectDestination());
				ip = bitmask24 & target;
			}
		}
		return advanceIp;
	}

DefOp(Branch) {
	switch (current.getBranchFlags()) {
		case 0b00:
			advanceIp = branchSpecificOperation<0b00>(getInstructionPointer(), getConditionRegister(), [this]() { return RegisterValue(getCurrentCodeWord()); }, [this](byte index) { return registerValue(index); }, std::move(current));
			break;
		case 0b01:
			advanceIp = branchSpecificOperation<0b01>(getInstructionPointer(), getConditionRegister(), [this]() { return RegisterValue(getCurrentCodeWord()); }, [this](byte index) { return registerValue(index); }, std::move(current));
			break;
		case 0b10:
			advanceIp = branchSpecificOperation<0b10>(getInstructionPointer(), getConditionRegister(), [this]() { return RegisterValue(getCurrentCodeWord()); }, [this](byte index) { return registerValue(index); }, std::move(current));
			break;
		case 0b11:
			advanceIp = branchSpecificOperation<0b11>(getInstructionPointer(), getConditionRegister(), [this]() { return RegisterValue(getCurrentCodeWord()); }, [this](byte index) { return registerValue(index); }, std::move(current));
			break;
		default:
			throw iris::Problem("Illegal flags combination!");
	}
}

DefOp(Call) {
	constexpr RegisterValue bitmask24 = mask<0b1111>();
	advanceIp = false;
	RegisterValue ip = getInstructionPointer();
	if (current.getCallFlagImmediate()) {
		++getInstructionPointer();
		// make a 24 bit number
		auto bottom = RegisterValue(current.getUpper());
		auto upper = RegisterValue(getCurrentCodeWord()) << 8;
		getInstructionPointer() = bitmask24 & (upper | bottom);
	} else {
		auto target = registerValue(current.getDestination());
		getInstructionPointer() = bitmask24 & target;
	}
	getLinkRegister() = ip + 1;
	if (getLinkRegister() > bitmask24) {
		getLinkRegister() &= bitmask24; // make sure that we aren't over the memory setup
	}
}

DefOp(If) {
	constexpr RegisterValue bitmask24 = mask<0b1111>();
	advanceIp = false;
	if (current.getIfFlagCall()) {
		getLinkRegister() = getInstructionPointer() + 1;
		if (getLinkRegister() > bitmask24) {
			getLinkRegister() &= bitmask24; // make sure that we aren't over the memory setup
		}
	} 
	RegisterValue addr = registerValue((getConditionRegister() != 0) ? current.getIfOnTrue() : current.getIfOnFalse());
	getInstructionPointer() = bitmask24 & addr; 
}

template<CompareCombine compareOp> 
bool combine(bool newValue, bool existingValue) {
	throw iris::Problem("Undefined combine operation");
}
template<>
bool combine<CompareCombine::None>(bool newValue, bool existingValue) {
	return newValue;
}

template<>
bool combine<CompareCombine::And>(bool newValue, bool existingValue) {
	return newValue && existingValue;
}

template<>
bool combine<CompareCombine::Or>(bool newValue, bool existingValue) {
	return newValue || existingValue;
}

template<>
bool combine<CompareCombine::Xor>(bool newValue, bool existingValue) {
	return newValue ^ existingValue;
}

template<CompareStyle style>
bool compare(RegisterValue a, RegisterValue b) {
	throw iris::Problem("Undefined comparison style!");
}

template<>
bool compare<CompareStyle::Equals>(RegisterValue a, RegisterValue b) {
	return a == b;
}

template<>
bool compare<CompareStyle::NotEquals>(RegisterValue a, RegisterValue b) {
	return a != b;
}

template<>
bool compare<CompareStyle::LessThan>(RegisterValue a, RegisterValue b) {
	return a < b;
}

template<>
bool compare<CompareStyle::GreaterThan>(RegisterValue a, RegisterValue b) {
	return a > b;
}

template<>
bool compare<CompareStyle::LessThanOrEqualTo>(RegisterValue a, RegisterValue b) {
	return a <= b;
}
template<>
bool compare<CompareStyle::GreaterThanOrEqualTo>(RegisterValue a, RegisterValue b) {
	return a >= b;
}

DefOp(Compare) {
	++getInstructionPointer();
	DecodedInstruction next(getCurrentCodeWord());
	switch (current.getConditionalCompareType()) {
#define X(type) \
		case CompareStyle:: type : { \
									   RegisterValue first = registerValue(next.getSrc1()); \
									   RegisterValue second = current.getConditionalImmediateFlag() ? next.getUpper() : registerValue(next.getSrc2()); \
									   bool result = compare<CompareStyle:: type>(first, second); \
									   switch (current.getConditionalCombineFlag()) { \
										   case CompareCombine::None: \
											   getConditionRegister() = combine<CompareCombine::None>(result, getConditionRegister()); \
											   break; \
										   case CompareCombine::And: \
											   getConditionRegister() = combine<CompareCombine::And>(result, getConditionRegister()); \
											   break; \
										   case CompareCombine::Or: \
											   getConditionRegister() = combine<CompareCombine::Or>(result, getConditionRegister()); \
											   break; \
										   case CompareCombine::Xor: \
											   getConditionRegister() = combine<CompareCombine::Xor>(result, getConditionRegister()); \
											   break; \
										   default: \
													throw iris::Problem("Illegal Compare Combine Operation"); \
									   } \
									   break; \
								   }
		X(Equals)
		X(NotEquals)
		X(LessThan)
		X(GreaterThan)
		X(LessThanOrEqualTo)
		X(GreaterThanOrEqualTo)
#undef X
		default:
			throw iris::Problem("illegal compare type!");
	}
}

DefOp(Return) {
	advanceIp = false;
	// jump to the link register
	getInstructionPointer() = getLinkRegister();
}

DefOp(Square) {
	registerValue(current.getDestination()) = registerValue(current.getDestination()) * registerValue(current.getDestination());
}

DefOp(Cube) {
	registerValue(current.getDestination()) = registerValue(current.getDestination()) * registerValue(current.getDestination()) * registerValue(current.getDestination());
}

	template<>
	void Core::operation<Operation::SystemCall>(DecodedInstruction&& current) {
		switch(static_cast<SystemCalls>(getAddressRegister())) {
			case SystemCalls::Terminate:
				execute = false;
				advanceIp = false;
				break;
			case SystemCalls::PutC:
				// read register 0 and register 1
				std::cout.put(static_cast<char>(registerValue(current.getDestination())));
				break;
			case SystemCalls::GetC:
				byte value;
				std::cin >> std::noskipws >> value;
				registerValue(current.getDestination()) = static_cast<word>(value);
				break;
			default:
				std::stringstream ss;
				ss << "Illegal system call " << std::hex << getAddressRegister();
				execute = false;
				advanceIp = false;
				throw iris::Problem(ss.str());
		}
	}

	void Core::dispatch(DecodedInstruction&& current) {
		auto controlValue = current.getControl();
		switch(controlValue) {
#define X(type) \
			case Operation:: type : \
				operation<Operation:: type>(std::move(current)); \
			break;
#include "def/iris17/ops.def"
#undef X
			default:
				std::stringstream str;
				str << "Illegal instruction " << std::hex << static_cast<byte>(controlValue);
				execute = false;
				throw iris::Problem(str.str());
		}
	}

	void Core::link(std::istream& input) {
		// two address system, 1 RegisterValue -> address, 1 word -> value
		constexpr int bufSize = sizeof(RegisterValue) + sizeof(word);
		char buf[bufSize] = { 0 };
		for(int lineNumber = 0; input.good(); ++lineNumber) {
			input.read(buf, bufSize);
			if (input.gcount() < bufSize && input.gcount() > 0) {
				throw iris::Problem("unaligned object file found!");
			} else if (input.gcount() == 0) {
				if (input.eof()) {
					break;
				} else {
					throw iris::Problem("something bad happened while reading input file!");
				}
			}
			//ignore the first byte, it is always zero
			RegisterValue address = encodeRegisterValue(buf[0], buf[1], buf[2], buf[3]);
			word value = encodeWord(buf[4], buf[5]);
			this->storeWord(address, value);
		}
	}
	RegisterValue& Core::registerValue(byte index) {
		if (index >= ArchitectureConstants::RegisterCount) {
			throw iris::Problem("Attempted to access an out of range register!");
		} else {
			return gpr[index];
		}
	}
	RegisterValue& Core::getInstructionPointer() {
		return registerValue<ArchitectureConstants::InstructionPointer>();
	}
	RegisterValue& Core::getStackPointer() {
		return registerValue<ArchitectureConstants::StackPointer>();
	}
	RegisterValue& Core::getConditionRegister() {
		return registerValue<ArchitectureConstants::ConditionRegister>();
	}
	RegisterValue& Core::getLinkRegister() {
		return registerValue<ArchitectureConstants::LinkRegister>();
	}
	RegisterValue& Core::getAddressRegister() {
		return registerValue<ArchitectureConstants::AddressRegister>();
	}
	RegisterValue& Core::getValueRegister() {
		return registerValue<ArchitectureConstants::ValueRegister>();
	}
	word Core::getCurrentCodeWord() {
		return memory[getInstructionPointer()];
	}
}

#include "iris20.h"
#include <functional>
#include <sstream>
#include <vector>

namespace iris20 {
	Core* newCore() noexcept {
		return new iris20::Core();
	}


	Core::~Core() {
	}

	void Core::installprogram(std::istream& stream) {
		auto encodeWord = [](char* buf) { return iris20::encodeWord(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]); };
		gpr.install(stream, encodeWord);
		memory.install(stream, encodeWord);
	}

	void Core::dump(std::ostream& stream) {

		auto decodeWord = [](word value, char* buf) { iris::decodeInt64LE(value, (byte*)buf); };
		gpr.dump(stream, decodeWord);
		memory.dump(stream, decodeWord);
	}
	void Core::run() {
		while(execute) {
			if (!advanceIp) {
				advanceIp = true;
			}
			dispatch();
			if (advanceIp) {
				++getInstructionPointer();
			}
		}
	}
	void Core::dispatch() {
		// TODO: add checks for dispatching on one or two atom molecules
		current = memory[getInstructionPointer()];
		if (decodeMoleculeContainsOneInstruction(current)) {
			executeMolecule();
		} else {
			executeAtom(getFirstAtom(current));
			executeAtom(getSecondAtom(current));
		}
	}
	enum class ExecutionUnitTarget {
		ALU,
		CompareUnit,
		BranchUnit,
		MiscUnit,
		MoveUnit,
	};
	// target, subcommand, immediate?
	using DispatchTableEntry = std::tuple<ExecutionUnitTarget, byte, bool>;
    constexpr DispatchTableEntry makeDispatchEntry(ExecutionUnitTarget target, byte value, bool immediate) {
        return std::make_tuple(target, value, immediate);
    }
    template<typename T>
    constexpr DispatchTableEntry makeDispatchEntry(ExecutionUnitTarget target, T value, bool immediate) {
        return makeDispatchEntry(target, static_cast<byte>(value), immediate);
    }
	constexpr inline byte makeJumpByte(bool ifthenelse, bool conditional, bool iffalse, bool link) noexcept {
		return iris::encodeFlag<byte, 0b00001000, 3>(
				iris::encodeFlag<byte, 0b00000100, 2>(
					iris::encodeFlag<byte, 0b00000010, 1>(
						iris::encodeFlag<byte, 0b00000001, 0>(0u,
							ifthenelse),
						conditional),
					iffalse),
				link);
	}
	constexpr inline DispatchTableEntry makeJumpConstant(bool ifthenelse, bool conditional, bool iffalse, bool immediate, bool link) noexcept {
        return makeDispatchEntry(ExecutionUnitTarget::BranchUnit, makeJumpByte(ifthenelse, conditional, iffalse, link), immediate);
	}
	constexpr inline std::tuple<bool, bool, bool, bool> decomposeJumpByte(byte input) noexcept {
		return std::make_tuple(iris::decodeFlag<byte, 0b00000001>(input), iris::decodeFlag<byte, 0b00000010>(input), iris::decodeFlag<byte, 0b00000100>(input), iris::decodeFlag<byte, 0b00001000>(input));
	}
    void Core::executeMolecule() {
        // decode the operation first!
        static std::map<Operation, DispatchTableEntry> table = {
            { Operation::Set32, makeDispatchEntry(ExecutionUnitTarget::MoveUnit, Operation::Set32, true) },
            { Operation::Set48, makeDispatchEntry(ExecutionUnitTarget::MoveUnit, Operation::Set48, true) },
        };
		auto result = table.find(decodeMoleculeOperation(current));
		if (result == table.end()) {
			throw iris::Problem("Illegal molecule instruction!");
		}
        ExecutionUnitTarget unit;
        byte dispatch;
        bool immediate;
        std::tie(unit, dispatch, immediate) = result->second;
        auto moveOperation = [this, op = static_cast<Operation>(dispatch), immediate]() {
            auto isSet = [](Operation op) { return op == Operation::Set32 || op == Operation::Set48; };
            auto getImmediateWord = [this](Operation op) {
                switch(op) {
                    case Operation::Set32:
                        return decodeImmediate32(current);
                    case Operation::Set48:
                        return decodeImmediate48(current);
                    default:
                        throw iris::Problem("Illegal operation to get a word from!");
                }
            };
            if (immediate) {
                if (isSet(op)) {
                    operandSet(decodeMoleculeDestination(current), getImmediateWord(op));
                } else {
                    throw iris::Problem("unimplemented move operation specified!");
                }
            } else {
                throw iris::Problem("no immediate operations currently defined!");
            }
        };
        switch(unit) {
            case ExecutionUnitTarget::MoveUnit:
                moveOperation();
                break;
            default:
                throw iris::Problem("Provided unit does not have molecule sized instructions!");

        }
    }
	void Core::executeAtom(InstructionAtom atom) {
		auto operation = getOperation(atom);
		static std::map<Operation, DispatchTableEntry> table = {
				{ Operation::Add, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Add) , false) },
				{ Operation::Sub, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Subtract) , false ) },
				{ Operation::Mul, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Multiply) , false ) } ,
				{ Operation::Div, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Divide) , false ) },
				{ Operation::Rem, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Remainder) , false ) },
				{ Operation::ShiftLeft, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::ShiftLeft) , false ) },
				{ Operation::ShiftRight, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::ShiftRight) , false ) },
				{ Operation::BinaryAnd, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::BinaryAnd) , false ) },
				{ Operation::BinaryOr, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::BinaryOr) , false ) },
				{ Operation::BinaryNot, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::UnaryNot) , false) },
				{ Operation::BinaryXor, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::BinaryXor) , false ) },
				{ Operation::AddImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Add) , true  ) },
				{ Operation::SubImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Subtract) , true  ) },
				{ Operation::MulImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Multiply) , true  ) } ,
				{ Operation::DivImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Divide) , true  ) },
				{ Operation::RemImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::Remainder) , true  ) },
				{ Operation::ShiftLeftImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::ShiftLeft) , true ) },
				{ Operation::ShiftRightImmediate, std::make_tuple(ExecutionUnitTarget::ALU, static_cast<byte>(ALU::Operation::ShiftRight) , true ) },
				{ Operation::LessThan, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::LessThan), false) },
				{ Operation::LessThanImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::LessThan), true) },
				{ Operation::LessThanOrEqualTo, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::LessThanOrEqualTo), false) },
				{ Operation::LessThanOrEqualToImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::LessThanOrEqualTo), true) },
				{ Operation::GreaterThan, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::GreaterThan), false) },
				{ Operation::GreaterThanImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::GreaterThan), true) },
				{ Operation::GreaterThanOrEqualTo, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::GreaterThanOrEqualTo), false) },
				{ Operation::GreaterThanOrEqualToImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::GreaterThanOrEqualTo), true) },
				{ Operation::Eq, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::Eq), false) },
				{ Operation::EqImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::Eq), true) },
				{ Operation::Neq, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::Neq), false) },
				{ Operation::NeqImm, std::make_tuple(ExecutionUnitTarget::CompareUnit, static_cast<byte>(CompareUnit::Operation::Neq), true) },
				{ Operation::SystemCall, std::make_tuple(ExecutionUnitTarget::MiscUnit, static_cast<byte>(Operation::SystemCall), false) },
				{ Operation:: UnconditionalImmediate ,        makeJumpConstant( false, false, false, true, false) } ,
				{ Operation:: UnconditionalImmediateLink ,    makeJumpConstant( false, false, false, true, true) } ,
				{ Operation:: UnconditionalRegister ,         makeJumpConstant( false, false, false, false, false) } ,
				{ Operation:: UnconditionalRegisterLink ,     makeJumpConstant( false, false, false, false, true) } ,
				{ Operation:: ConditionalTrueImmediate ,      makeJumpConstant( false, true, false, true, false) } ,
				{ Operation:: ConditionalTrueImmediateLink ,  makeJumpConstant( false, true, false, true, true) } ,
				{ Operation:: ConditionalTrueRegister ,       makeJumpConstant( false, true, false, false, false) } ,
				{ Operation:: ConditionalTrueRegisterLink ,   makeJumpConstant( false, true, false, false, true) } ,
				{ Operation:: ConditionalFalseImmediate ,     makeJumpConstant( false, true, true, true, false) } ,
				{ Operation:: ConditionalFalseImmediateLink , makeJumpConstant( false, true, true, true, true) } ,
				{ Operation:: ConditionalFalseRegister ,      makeJumpConstant( false, true, true, false, false) } ,
				{ Operation:: ConditionalFalseRegisterLink ,  makeJumpConstant( false, true, true, false, true) } ,
				{ Operation:: IfThenElseNormalPredTrue ,      makeJumpConstant( true, true, false, false, false) } ,
				{ Operation:: IfThenElseNormalPredFalse ,     makeJumpConstant( true, true, true, false, false) } ,
				{ Operation:: IfThenElseLinkPredTrue ,        makeJumpConstant( true, true, false, false, true) } ,
				{ Operation:: IfThenElseLinkPredFalse ,       makeJumpConstant( true, true, true, false, true) } ,
				{ Operation::Move, std::make_tuple(ExecutionUnitTarget::MoveUnit, static_cast<byte>(Operation::Move), false) },
				{ Operation::Swap, std::make_tuple(ExecutionUnitTarget::MoveUnit, static_cast<byte>(Operation::Swap), false) },
				{ Operation::Set16, std::make_tuple(ExecutionUnitTarget::MoveUnit, static_cast<byte>(Operation::Set16), true) },
		};
		auto result = table.find(operation);
		if (result == table.end()) {
			throw iris::Problem("Illegal single atom instruction!");
		}
		auto tuple = result->second;
		auto target = std::get<ExecutionUnitTarget>(tuple);
		auto subAction = std::get<byte>(tuple);
		auto immediate = std::get<bool>(tuple);
		auto moveOperation = [this, op = static_cast<Operation>(subAction), immediate, atom]() {
			auto dest = getDestinationRawValue(atom);
			auto src = getSource0RawValue(atom);
			if (op == Operation::Move) {
				operandSet(dest, operandGet(src));
			} else if (op == Operation::Set16) {
				operandSet(dest, getImmediate(atom));
			} else if (op == Operation::Swap) {
				auto tmp = operandGet(dest);
				operandSet(dest, operandGet(src));
				operandSet(src, tmp);
			} else {
				throw iris::Problem("Registered but unimplemented move unit operation!");
			}
		};
		auto miscOperation = [this, operation, immediate, atom]() {
			if (operation == Operation::SystemCall) {
				auto sysCallId = static_cast<SystemCalls>(getDestinationRawValue(atom));
				if (sysCallId == SystemCalls::Terminate) {
					execute = false;
					advanceIp = false;
				} else if (sysCallId == SystemCalls::PutC) {
					// read register 0 and register 1
					std::cout.put(static_cast<char>(operandGet(getSource0RawValue(atom))));
				} else if (sysCallId == SystemCalls::GetC) {
					auto value = static_cast<byte>(0);
					std::cin >> std::noskipws >> value;
					operandSet(getSource0RawValue(atom), static_cast<word>(value));
				} else {
					std::stringstream stream;
					stream << "Illegal system call " << std::hex << getDestinationRawValue(atom);
					execute = false;
					advanceIp = false;
					throw iris::Problem(stream.str());
				}
			} else {
				throw iris::Problem("Registered but undefined misc operation requested!");
			}
		};
		auto jumpOperation = [this, subAction, immediate, atom]() {
			auto ifthenelse = false, conditional = false, iffalse = false, link = false;
			auto result = decomposeJumpByte(subAction);
			std::tie(ifthenelse, conditional, iffalse, link) = result;
			auto newAddr = static_cast<word>(0);
			auto cond = true;
			advanceIp = false;
			auto ip = getInstructionPointer();
			auto dest = operandGet(getDestinationRawValue(atom));
			auto src0Ind = getSource0RawValue(atom);
			auto src1Ind = getSource0RawValue(atom);
			if (conditional) {
				cond = (iffalse ? (dest == 0) : (dest != 0));
				if (ifthenelse) {
					newAddr = operandGet(cond ? src0Ind : src1Ind);
				} else {
					newAddr = cond ? (immediate ? getImmediate(atom) : operandGet(src1Ind)) : ip + 1;
				}
			} else {
				newAddr = immediate ? getImmediate(atom) : dest;
			}
			getInstructionPointer() = newAddr;
			if (link && cond) {
				getLinkRegister() = ip + 1;
			}

		};
		switch (target) {
			case ExecutionUnitTarget::ALU:
				performOperation(_alu, static_cast<ALU::Operation>(subAction), immediate, atom);
				break;
			case ExecutionUnitTarget::CompareUnit:
				performOperation(_compare, static_cast<CompareUnit::Operation>(subAction), immediate, atom);
				break;
			case ExecutionUnitTarget::MiscUnit:
				miscOperation();
				break;
			case ExecutionUnitTarget::BranchUnit:
				jumpOperation();
				break;
			case ExecutionUnitTarget::MoveUnit:
				moveOperation();
				break;
			default:
				throw iris::Problem("Registered execution unit target is not yet implemented!");
		}
	}

	void Core::link(std::istream& input) {
		constexpr auto bufSize = sizeof(word) * 2;
		char buf[bufSize] = { 0 };
		for(auto lineNumber = static_cast<int>(0); input.good(); ++lineNumber) {
			input.read(buf, bufSize);
			if (input.gcount() < bufSize && input.gcount() > 0) {
				throw iris::Problem("unaligned object file found!");
			} else if (input.gcount() == 0) {
				if (input.eof()) {
					break;
				} else {
					throw iris::Problem("Something bad happened while reading input file!");
				}
			}
			// first 8 bytes are an address, second 8 are a value
			auto address = iris20::encodeWord(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
			auto value = iris20::encodeWord(buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
			if (debugEnabled()) {
				std::cerr << "addr: 0x " << std::hex << address << ": value: 0x" << std::hex << value << std::endl;
			}
			memory[address] = value;
		}
	}

	Core::Core() noexcept { }

	void Core::initialize() {
		memory.zero();
	}

    void Core::operandSet(byte target, word value) {
        SectionType type;
        byte index;
        std::tie(type, index) = getOperand(target);
        auto& data = gpr[index];
        auto pushValue = [this, &data](word value) {
            --data;
            memory[data] = value;
        };
        switch(type) {
            case SectionType::Register:
                data = value;
                break;
            case SectionType::Memory:
                memory[data] = value;
                break;
            case SectionType::Stack:
                pushValue(value);
                break;
            default:
                throw iris::Problem("Undefined section type specified!");
        }
    }

    word Core::operandGet(byte target) {
        SectionType type;
        byte index;
        std::tie(type, index) = getOperand(target);
        auto& data = gpr[index];
        auto popData = [this, &data]() {
            auto outcome = memory[data];
            ++data;
            return outcome;
        };
        auto loadData = [this, &data]() {
            return memory[data];
        };
        switch(type) {
            case SectionType::Register:
                return data;
            case SectionType::Stack:
                return popData();
            case SectionType::Memory:
                return loadData();
            default:
                throw iris::Problem("Undefined section type specified!");
        }
    }

}
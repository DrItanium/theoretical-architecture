// Cisc0CoreAssembler rewritten to use pegtl
#include <string>
#include <sstream>
#include <typeinfo>
#include <iostream>
#include <map>
#include "Base.h"
#include "AssemblerBase.h"
#include "Problem.h"
#include "Cisc0Core.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/file_parser.hh>
#include <pegtl/contrib/raw_string.hh>
#include <pegtl/contrib/abnf.hh>
#include <pegtl/parse.hh>
#include <vector>

namespace cisc0 {
    using Separator = syn::AsmSeparator;
    using SingleLineComment = syn::SingleLineComment<';'>;
    template<typename R> struct Action : pegtl::nothing<R> { };
    void reportError(const std::string& msg) {
        throw syn::Problem(msg);
    }

    using AssemblerWord = syn::AssemblerWord<RegisterValue>;
    struct AssemblerState {
        cisc0::Address currentAddress = 0;
        std::vector<InstructionEncoder> finishedInstructions;
        std::map<std::string, RegisterValue> labels;
        std::vector<AssemblerWord> finalWords;
        std::vector<AssemblerWord> wordsToResolve;
        void setCurrentAddress(Address addr) noexcept;
        void output(std::ostream* out) noexcept;
        void resolveInstructions();
        void resolveDeclarations();
    };
    template<int width>
    struct AssemblerWordCreator {
            static_assert(width > 0, "Can't have a width of zero or less!");
            template<typename Input>
            AssemblerWordCreator(const Input& in, AssemblerState& parent) { }
            virtual ~AssemblerWordCreator() { }
            template<typename Input>
            void success(const Input& in, AssemblerState& parent) {
                if (_isLabel) {
                    parent.wordsToResolve.emplace_back(parent.currentAddress, _label, width);
                } else {
                    parent.wordsToResolve.emplace_back(parent.currentAddress, _value, width);
                }
                parent.currentAddress += width;
            }
            bool _isLabel;
            std::string _label;
            RegisterValue _value;

    };
    using WordCreator = AssemblerWordCreator<1>;
    using DwordCreator = AssemblerWordCreator<2>;

    struct ChangeCurrentAddress {
        template<typename Input>
        ChangeCurrentAddress(const Input& in, AssemblerState& parent) { }

        template<typename Input>
        void success(const Input& in, AssemblerState& parent) {
             parent.currentAddress = address;
        }
        RegisterValue address;
    };
    struct RegisterLabel {
        template<typename Input>
        RegisterLabel(const Input& in, AssemblerState& parent) : _address(parent.currentAddress) {
        }

        template<typename Input>
        void success(const Input& in, AssemblerState& parent) {
            parent.labels.emplace(_title, _address);
        }
        Address _address;
        std::string _title;

    };
    struct AssemblerInstruction : public InstructionEncoder {
        template<typename Input>
            AssemblerInstruction(const Input& in, AssemblerState& parent) {
                address = parent.currentAddress;
            }

        template<typename Input>
            void success(const Input& in, AssemblerState& parent) {
                parent.currentAddress += numWords();
                // for now, make a copy because I do not care!
                parent.finishedInstructions.push_back(*this);
            }
    };

    struct NumberContainer {
        template<typename Input, typename ... States>
        NumberContainer(const Input& in, States && ...) { }

        template<typename Input>
        void success(const Input& in, AssemblerInstruction& parent) {
            parent.fullImmediate = _value;
        }

        template<typename Input>
        void success(const Input& in, RegisterLabel& parent) {
            parent._address = _value;
        }

        template<typename Input>
        void success(const Input& in, ChangeCurrentAddress& parent) {
            parent.address = _value;
        }

        template<typename Input>
        void success(const Input& in, AssemblerWord& parent) {
            parent.setValue(_value);
        }

        template<typename Input>
        void success(const Input& in, WordCreator& parent) {
            parent._value = _value;
            parent._isLabel = false;
        }

        template<typename Input>
        void success(const Input& in, DwordCreator& parent) {
            parent._value = _value;
            parent._isLabel = false;
        }

        RegisterValue _value;
    };
    void AssemblerState::resolveInstructions() {
        for (auto & op : finishedInstructions) {
            if (op.isLabel) {
                auto label = op.labelValue;
                auto f = labels.find(label);
                if (f == labels.end()) {
                    std::stringstream stream;
                    stream << "label " << label << " does not exist!\n";
                    throw syn::Problem(stream.str());
                }
                op.fullImmediate = f->second;
            }
            // now that it has been resolved, we need to go through and setup
            // the encoding correctly!
            auto address = op.address;
            int count;
            Word first, second, third;
            std::tie(count, first, second, third) = op.encode();
            //std::cerr << "count = " << count << std::endl;
            //std::cerr << "- first = " << std::hex << first << std::endl;
            //std::cerr << "- second = " << std::hex << second << std::endl;
            //std::cerr << "- third = " << std::hex << third << std::endl;
            switch(count) {
                case 3:
                    finalWords.emplace_back(address + 2, third);
                case 2:
                    finalWords.emplace_back(address + 1, second);
                case 1:
                    finalWords.emplace_back(address, first);
                    break;
                default:
                    throw syn::Problem("Number of words described is not possible!");
            }
        }
    }
    void AssemblerState::resolveDeclarations() {
        for (auto & op: wordsToResolve) {
            if (op.isLabel()) {
                auto label = op.getLabel();
                auto f = labels.find(label);
                if (f == labels.end()) {
                    std::stringstream stream;
                    stream << "label " << label << " does not exist!\n";
                    throw syn::Problem(stream.str());
                }
                op.setValue(f->second);
            }
            switch(op.getWidth()) {
                case 2:
                    finalWords.emplace_back(op.getAddress() + 1, syn::getUpperHalf(op.getValue()));
                case 1:
                    finalWords.emplace_back(op.getAddress(), syn::getLowerHalf(op.getValue()));
                    break;
                default:
                    throw syn::Problem("Got a declaration of with a width that was not 1 or 2");
            }
        }
    }

#define DefSymbol(title, str) \
    struct Symbol ## title : public pegtl_string_t( #str ) { }
#define DefAction(rule) template<> struct Action < rule >
#define DefApplyGeneric(type) template<typename Input> static void apply(const Input& in, type& state)
#define DefApplyInstruction DefApplyGeneric(cisc0::AssemblerInstruction)
#define DefApplyAsmState DefApplyGeneric(cisc0::AssemblerState)

#define DefGroup(title, str) \
    DefSymbol(title, str); \
    struct Group ## title : syn::Indirection<Symbol ## title> { }; \
    DefAction(Group ## title) { \
        DefApplyInstruction { \
            state.type = Operation:: title ; \
        } \
    }
    // done
    DefGroup(Shift, shift);
    DefGroup(Compare, compare);
    DefGroup(Move, move);
    DefGroup(Set, set);
    DefGroup(Swap, swap);
    DefGroup(SystemCall, system);
    DefGroup(Arithmetic, arithmetic);
    DefGroup(Memory, memory);
    DefGroup(Logical, logical);
    DefGroup(Complex, complex);
    DefGroup(Branch, branch);

    //DefSymbol(Nop, nop);
    //DefSymbol(Return, return);
    DefSymbol(Immediate, immediate);



    struct UsesImmediate : pegtl::seq<SymbolImmediate> { };

    DefAction(UsesImmediate) {
        DefApplyInstruction {
            state.immediate = true;
        }
    };

    using HexadecimalNumber = syn::HexadecimalNumber;
    DefAction(HexadecimalNumber) {
        static auto parseHex(const std::string& str) {
            return syn::getHexImmediate<RegisterValue>(str, reportError);
        }
        DefApplyGeneric(NumberContainer) {
            state._value = parseHex(in.string());
        }
    };
    using BinaryNumber = syn::BinaryNumber;
    DefAction(BinaryNumber) {
        static auto parseBinary(const std::string& str) {
            return syn::getBinaryImmediate<RegisterValue>(str, reportError);
        }
        DefApplyGeneric(NumberContainer) {
            state._value = parseBinary(in.string());
        }
    };
    using DecimalNumber = syn::Base10Number;
    DefAction(DecimalNumber) {
        static auto parseDecimalImmediate(const std::string& input) {
            return syn::getDecimalImmediate<RegisterValue>(input.c_str(), reportError);
        }
        DefApplyGeneric(NumberContainer) {
            state._value = parseDecimalImmediate(in.string());
        }
    };
    struct Number : public pegtl::state<NumberContainer, pegtl::sor<HexadecimalNumber, DecimalNumber, BinaryNumber > > { };
    DefAction(Number) {
        DefApplyInstruction {
            state.isLabel = false;
        }
        DefApplyGeneric(ChangeCurrentAddress) {
        }
    };

    struct BitmaskNumber : syn::GenericNumeral<'m', pegtl::abnf::BIT> { };

    DefAction(BitmaskNumber) {
        DefApplyInstruction {
            state.bitmask = syn::decodeBits<RegisterValue, byte, 0x000000FF, 0>(syn::getBinaryImmediate<RegisterValue>(in.string(), reportError));
        }
    };
    using Lexeme = syn::Lexeme;
    DefAction(Lexeme) {
        DefApplyInstruction {
            state.labelValue = in.string();
            state.fullImmediate = 0;
            state.isLabel = true;
        }
        DefApplyGeneric(RegisterLabel) {
            state._title = in.string();
        }
        DefApplyGeneric(WordCreator) {
            state._label = in.string();
            state._isLabel = true;
        }
        DefApplyGeneric(DwordCreator) {
            state._label = in.string();
            state._isLabel = true;
        }
    };
    struct LexemeOrNumber : public syn::LexemeOr<Number> { };

    struct NormalRegister : public syn::GenericRegister<'r'> { };
    DefSymbol(AddrRegister, addr);
    DefSymbol(StackPointer, sp);
    DefSymbol(InstructionPointer, ip);
    DefSymbol(ConditionRegister, cr);
    DefSymbol(ValueRegister, value);
    DefSymbol(MaskRegister, mask);
    DefSymbol(FieldRegister, field);
    struct GeneralPurposeRegister : pegtl::sor<
                                    NormalRegister,
                                    SymbolAddrRegister,
                                    SymbolStackPointer,
                                    SymbolInstructionPointer,
                                    SymbolConditionRegister,
                                    SymbolValueRegister,
                                    SymbolMaskRegister,
                                    SymbolFieldRegister> { };
    Word translateRegister(const std::string& input) {
        static std::map<std::string, Word> builtinAliases = {
            { "addr", static_cast<Word>(ArchitectureConstants::AddressRegister) },
            { "ip", static_cast<Word>(ArchitectureConstants::InstructionPointer) },
            { "sp", static_cast<Word>(ArchitectureConstants::StackPointer) },
            { "value", static_cast<Word>(ArchitectureConstants::ValueRegister) },
            { "mask", static_cast<Word>(ArchitectureConstants::MaskRegister) },
            { "shift", static_cast<Word>(ArchitectureConstants::ShiftRegister) },
            { "field", static_cast<Word>(ArchitectureConstants::FieldRegister) },
        };
        auto result = builtinAliases.find(input);
        if (result == builtinAliases.end()) {
            return syn::getRegister<Word, ArchitectureConstants::RegisterCount>(input, reportError);
        } else {
            return result->second;
        }
    }

    using IndirectGPR = syn::Indirection<GeneralPurposeRegister>;
#define DefIndirectGPR(title) \
    struct title : public IndirectGPR { }

    DefIndirectGPR(DestinationRegister);
    DefAction(DestinationRegister) {
        DefApplyInstruction {
            state.arg0 = translateRegister(in.string());
        }
    };

    DefIndirectGPR(SourceRegister);
    DefAction(SourceRegister) {
        DefApplyInstruction {
            state.arg1 = translateRegister(in.string());
        }
    };

    DefIndirectGPR(SourceRegister1);
    DefAction(SourceRegister1) {
        DefApplyInstruction {
            state.arg2 = translateRegister(in.string());
        }
    };
    template<typename S>
        struct TwoArgumentOperation : pegtl::seq<DestinationRegister, Separator, S> { };
    struct TwoGPRs : TwoArgumentOperation<SourceRegister> { };
    DefAction(TwoGPRs) {
        DefApplyInstruction {
            state.immediate = false;
        }
    };

    DefSymbol(Left, left);
    DefSymbol(Right, right);

    struct ShiftLeftOrRight : pegtl::sor<SymbolLeft, SymbolRight> { };

    DefAction(ShiftLeftOrRight) {
        DefApplyInstruction {
            state.shiftLeft = (in.string() == "left");
        }
    };

    template<typename Source>
        struct ImmediateOperationArgs : pegtl::seq<UsesImmediate, Separator, TwoArgumentOperation<Source>> { };
    template<typename Source>
        struct ImmediateOperationArgsWithBitmask : pegtl::seq<UsesImmediate, Separator, BitmaskNumber, Separator, TwoArgumentOperation<Source>> { };

    struct ShiftImmediateValue : pegtl::seq<Number> { };
    DefAction(ShiftImmediateValue) {
        DefApplyInstruction {
            state.arg1 = static_cast<byte>(state.fullImmediate) & 0b11111;
        }
    };
    struct ShiftArgs : pegtl::sor<TwoGPRs, ImmediateOperationArgs<ShiftImmediateValue>> { };

    struct ShiftOperation : pegtl::seq<GroupShift, Separator, ShiftLeftOrRight, Separator, ShiftArgs> { };

    struct ByteCastImmediate : pegtl::seq<Number> { };
    DefAction(ByteCastImmediate) {
        DefApplyInstruction {
            state.arg1 = static_cast<byte>(state.fullImmediate);
        }
    };
#define DefSubType(title, str, subgroup) \
    struct SubGroup ## subgroup ## title : syn::Indirection<Symbol ## title> { }; \
    DefAction(SubGroup ## subgroup ## title) { \
        DefApplyInstruction { \
            state.subType = static_cast < decltype(state.subType) > ( cisc0 :: subgroup :: title ) ; \
        } \
    }

#define DefSubTypeWithSymbol(title, str, subgroup) \
    DefSymbol(title, str); \
    DefSubType(title, str, subgroup)

#define DefCompareStyle(title, str) DefSubType(title, str, CompareStyle)

#define DefCompareStyleWithSymbol(title, str) DefSubTypeWithSymbol(title, str, CompareStyle)

    DefCompareStyleWithSymbol(Equals, ==);
    DefCompareStyleWithSymbol(NotEquals, !=);
    DefCompareStyleWithSymbol(LessThan, <);
    DefCompareStyleWithSymbol(LessThanOrEqualTo, <=);
    DefCompareStyleWithSymbol(GreaterThan, >);
    DefCompareStyleWithSymbol(GreaterThanOrEqualTo, >=);
    struct CompareType : pegtl::sor<
                         SubGroupCompareStyleEquals,
                         SubGroupCompareStyleNotEquals,
                         SubGroupCompareStyleLessThan,
                         SubGroupCompareStyleLessThanOrEqualTo,
                         SubGroupCompareStyleGreaterThan,
                         SubGroupCompareStyleGreaterThanOrEqualTo> { };
    struct CompareArgs : pegtl::sor<TwoGPRs, ImmediateOperationArgs<ByteCastImmediate>> { };
    struct CompareOperation : pegtl::seq<GroupCompare, Separator, CompareType, Separator, CompareArgs> { };
    struct MoveOperation : pegtl::seq<
                           GroupMove,
                           Separator,
                           BitmaskNumber,
                           Separator,
                           TwoGPRs> { };
    struct SetOperation : pegtl::seq<
                          GroupSet,
                          Separator,
                          BitmaskNumber,
                          Separator,
                          DestinationRegister,
                          Separator,
                          LexemeOrNumber> { };

    struct SwapOperation : pegtl::seq<
                           GroupSwap,
                           Separator,
                           TwoGPRs> { };

    struct Arg0ImmediateValue : pegtl::seq<Number> { };
    DefAction(Arg0ImmediateValue) {
        DefApplyInstruction {
            state.arg0 = static_cast<byte>(state.fullImmediate) & 0b1111;
        }
    };
    struct SystemCallOperation : pegtl::seq<
                                 GroupSystemCall,
                                 Separator,
                                 DestinationRegister> { };
#define DefArithmeticOperation(title, str) \
    DefSubTypeWithSymbol(title, str, ArithmeticOps)

    DefArithmeticOperation(Add, add);
    DefArithmeticOperation(Sub, sub);
    DefArithmeticOperation(Mul, mul);
    DefArithmeticOperation(Div, div);
    DefArithmeticOperation(Rem, rem);
    struct ArithmeticType : pegtl::sor<
                            SubGroupArithmeticOpsAdd,
                            SubGroupArithmeticOpsSub,
                            SubGroupArithmeticOpsMul,
                            SubGroupArithmeticOpsDiv,
                            SubGroupArithmeticOpsRem> { };

    struct ArithmeticArgs : pegtl::sor<
                            TwoGPRs,
                            ImmediateOperationArgs<ByteCastImmediate>> { };
    struct ArithmeticOperation : pegtl::seq<
                                 GroupArithmetic,
                                 Separator,
                                 ArithmeticType,
                                 Separator,
                                 ArithmeticArgs> { };

#define DefMemoryOperation(title, str) \
    DefSubTypeWithSymbol(title, str, MemoryOperation)
    DefMemoryOperation(Load, load);
    DefMemoryOperation(Store, store);
    DefMemoryOperation(Push, push);
    DefMemoryOperation(Pop, pop);

    struct LoadStoreType : pegtl::sor<
                           SubGroupMemoryOperationLoad,
                           SubGroupMemoryOperationStore> { };
    struct StackMemoryType : pegtl::sor<
                             SubGroupMemoryOperationPush,
                             SubGroupMemoryOperationPop> { };
    struct StackOperation : pegtl::seq<
                            StackMemoryType,
                            Separator,
                            BitmaskNumber,
                            Separator,
                            DestinationRegister> { };
    DefSymbol(Indirect, indirect);
    struct FlagIndirect : public syn::Indirection<SymbolIndirect> { };
    DefAction(FlagIndirect) {
        DefApplyInstruction {
            state.indirect = true;
        }
    };
    DefSymbol(Direct, direct);
    struct FlagDirect : public syn::Indirection<SymbolDirect> { };
    DefAction(FlagDirect) {
        DefApplyInstruction {
            state.indirect = false;
        }
    };
    struct FlagDirectOrIndirect : pegtl::sor<FlagDirect, FlagIndirect> { };
    struct LoadStoreOperation : pegtl::seq<
                                LoadStoreType,
                                Separator,
                                BitmaskNumber,
                                Separator,
                                FlagDirectOrIndirect,
                                Separator,
                                Arg0ImmediateValue> { };

    struct MemoryInstruction : pegtl::seq<
                               GroupMemory,
                               Separator,
                               pegtl::sor<
                               StackOperation,
                               LoadStoreOperation>> { };

#define DefLogicalOperation(title, str) \
    DefSubTypeWithSymbol(title, str, LogicalOps)
    DefLogicalOperation(And, and);
    DefLogicalOperation(Or, or);
    DefLogicalOperation(Not, not);
    DefLogicalOperation(Xor, xor);
    DefLogicalOperation(Nand, nand);


    struct LogicalOpsType : pegtl::sor<
                            SubGroupLogicalOpsAnd,
                            SubGroupLogicalOpsOr,
                            SubGroupLogicalOpsNot,
                            SubGroupLogicalOpsXor,
                            SubGroupLogicalOpsNand> { };
    struct LogicalArgs : pegtl::sor<
                         TwoGPRs,
                         ImmediateOperationArgsWithBitmask<LexemeOrNumber>> { };
    struct LogicalOperation : pegtl::seq<
                              GroupLogical,
                              Separator,
                              LogicalOpsType,
                              Separator,
                              LogicalArgs> { };



#define DefEncodingSubType(title, str) \
    DefSubTypeWithSymbol(title, str, EncodingOperation)
    DefEncodingSubType(BitSet, bitset);
    DefEncodingSubType(BitUnset, bitunset);
    DefEncodingSubType(Encode, encode);
    DefEncodingSubType(Decode, decode);
    struct ComplexEncodingSubOperation : pegtl::sor<
                                         SubGroupEncodingOperationDecode,
                                         SubGroupEncodingOperationEncode,
                                         SubGroupEncodingOperationBitSet,
                                         SubGroupEncodingOperationBitUnset> { };
#define DefComplexOperation(title, str) \
    DefSubTypeWithSymbol(title, str, ComplexSubTypes)
    DefComplexOperation(Encoding, encoding);


    struct ComplexEncodingOperation : pegtl::seq<
                                      SubGroupComplexSubTypesEncoding,
                                      Separator,
                                      ComplexEncodingSubOperation> { };
    struct ComplexSubOperations : pegtl::sor<
                                  ComplexEncodingOperation> { };

    struct ComplexOperation : pegtl::seq<
                              GroupComplex,
                              Separator,
                              ComplexSubOperations> { };

    DefSymbol(If, if);
    DefSymbol(Call, call);
    DefSymbol(NoCall, nocall);
    DefSymbol(Conditional, conditional);
    DefSymbol(Unconditional, unconditional);

    template<typename T, typename F>
        struct ChoiceFlag : pegtl::sor<T, F> { };

    struct BranchFlagIf : public syn::Indirection<SymbolIf> { };
    DefAction(BranchFlagIf) {
        DefApplyInstruction {
            state.isIf = true;
            state.isConditional = false;
        }
    };

    struct BranchFlagCall : public syn::Indirection<SymbolCall> { };
    DefAction(BranchFlagCall) {
        DefApplyInstruction {
            state.isCall = true;
        }
    };

    struct BranchFlagNoCall : public syn::Indirection<SymbolNoCall> { };
    DefAction(BranchFlagNoCall) {
        DefApplyInstruction {
            state.isCall = false;
        }
    };

    struct ChooseBranchFlagCall : ChoiceFlag<BranchFlagCall, BranchFlagNoCall> { };

    struct BranchFlagConditional : public syn::Indirection<SymbolConditional> { };
    DefAction(BranchFlagConditional) {
        DefApplyInstruction {
            state.isConditional = true;
        }
    };

    struct BranchFlagUnconditional : public syn::Indirection<SymbolUnconditional> { };
    DefAction(BranchFlagUnconditional) {
        DefApplyInstruction {
            state.isConditional = false;
        }
    };

    struct ChooseBranchFlagUsePredicate : ChoiceFlag<BranchFlagConditional, BranchFlagUnconditional> { };

    struct BranchIfOperation : pegtl::seq<
                               BranchFlagIf,
                               Separator,
                               ChooseBranchFlagCall,
                               Separator,
                               TwoGPRs> { };
    struct BranchNormalArgs : pegtl::sor<
                              pegtl::seq<
                              UsesImmediate,
                              Separator,
                              LexemeOrNumber>,
                              DestinationRegister> { };
    struct BranchCallOperation : pegtl::seq<
                                 BranchFlagCall,
                                 Separator,
                                 BranchNormalArgs> { };
    struct BranchJumpOperation : pegtl::seq<
                                 ChooseBranchFlagUsePredicate,
                                 Separator,
                                 BranchNormalArgs> { };

    struct BranchOperation : pegtl::seq<
                             GroupBranch,
                             Separator,
                             pegtl::sor<BranchIfOperation,
                             BranchCallOperation,
                             BranchJumpOperation>> { };

    struct Instructions : pegtl::state<AssemblerInstruction,
    pegtl::sor<
               BranchOperation,
               ComplexOperation,
               MemoryInstruction,
               MoveOperation,
               SetOperation,
               SwapOperation,
               ArithmeticOperation,
               ShiftOperation,
               CompareOperation,
               SystemCallOperation,
               LogicalOperation>
                          > { };
    template<typename Symbol, typename Value>
        struct SingleArgumentDirective : pegtl::seq<Symbol, Separator, Value> { };

    DefSymbol(Org, .org);
    struct OrgDirective : pegtl::state<ChangeCurrentAddress, SingleArgumentDirective<SymbolOrg, Number>> { };

    DefSymbol(Label, .label);
    struct LabelDirective : pegtl::state<RegisterLabel, SingleArgumentDirective<SymbolLabel, Lexeme>> { };

    DefSymbol(Word, .word);
    DefSymbol(Dword, .dword);
    struct WordDirective : pegtl::state<WordCreator, SingleArgumentDirective<SymbolWord, LexemeOrNumber>> { };
    struct DwordDirective : pegtl::state<DwordCreator, SingleArgumentDirective<SymbolDword, LexemeOrNumber>> { };


    struct Directive : pegtl::sor<
                       OrgDirective,
                       LabelDirective
                       //WordDirective,
                       //DwordDirective
                       > { };

    struct Statement : pegtl::sor<
                       Instructions,
                       Directive> { };
    struct Anything : pegtl::sor<
                      Separator,
                      SingleLineComment,
                      Statement> { };

    struct Main : public syn::MainFileParser<Anything> { };

    void assemble (const std::string& iName, FILE* input, std::ostream* output) {
        pegtl::analyze<cisc0::Main>();
        AssemblerState as;
        pegtl::parse_cstream<cisc0::Main, cisc0::Action>(input, iName.c_str(), 16777216, as);
        // then go through and resolve everything!
        as.resolveDeclarations();
        as.resolveInstructions();
        as.output(output);
    }

    void AssemblerState::output(std::ostream* out) noexcept {
        char buf[8] = { 0 };
        for(auto const & address : finalWords) {
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = static_cast<char>(address.getAddress());
            buf[3] = static_cast<char>(address.getAddress() >> 8);
            buf[4] = static_cast<char>(address.getAddress() >> 16);
            buf[5] = static_cast<char>(address.getAddress() >> 24);
            buf[6] = static_cast<char>(address.getValue());
            buf[7] = static_cast<char>(address.getValue() >> 8);
            out->write(buf, 8);
        }
    }
}
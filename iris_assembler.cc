// iris_assembler rewritten to use pegtl
#include "iris.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/file_parser.hh>
#include <pegtl/contrib/raw_string.hh>

namespace iris {
    struct Comment : pegtl::until<pegtl::eolf > { };
    struct SymbolAdd : public pegtl_string_t( "add" ) { };
    struct SymbolSub : public pegtl_string_t( "sub" ) { };
    struct SymbolMul : public pegtl_string_t( "mul" ) { };
    struct SymbolDiv : public pegtl_string_t( "div" ) { };
    struct SymbolRem : public pegtl_string_t( "rem" ) { };
    struct SymbolShiftLeft : public pegtl_string_t("shl") { };
    struct SymbolShiftRight : public pegtl_string_t("shr") { };
    struct SymbolAnd : public pegtl_string_t("and") { };
    struct SymbolOr : public pegtl_string_t("or") { };
    struct SymbolNot : public pegtl_string_t("not") { };
    struct SymbolXor : public pegtl_string_t("xor") { };
    struct SymbolAddImmediate : public pegtl_string_t( "addi" ) { };
    struct SymbolSubImmediate : public pegtl_string_t( "subi" ) { };
    struct SymbolMulImmediate : public pegtl_string_t( "muli" ) { };
    struct SymbolDivImmediate : public pegtl_string_t( "divi" ) { };
    struct SymbolRemImmediate : public pegtl_string_t( "remi" ) { };
    struct SymbolShiftLeftImmediate : public pegtl_string_t("shli") { };
    struct SymbolShiftRightImmediate : public pegtl_string_t("shri") { };
    struct SymbolMove : public pegtl_string_t("move") { };
    struct SymbolSwap : public pegtl_string_t("swap") { };
    struct SymbolSet : public pegtl_string_t("set") { };
    struct SymbolLoad : public pegtl_string_t("ld") { };
    struct SymbolLoadImmediate : public pegtl_string_t("ldi") { };
    struct SymbolLoadMemory : public pegtl_string_t("ldm") { };
    struct SymbolLoadWithOffset : public pegtl_string_t("ldwo") { };
    struct SymbolLoadCode : public pegtl_string_t("ldc") { };
    struct SymbolStore : public pegtl_string_t("st") { };
    struct SymbolStoreImmediate : public pegtl_string_t("memset") { };
    struct SymbolStoreWithOffset : public pegtl_string_t("stwo") { };
    struct SymbolStoreCode : public pegtl_string_t("stc") { };
#define DefSymbol(title, str) \
    struct Symbol ## title : public pegtl_string_t( #str ) { }

    DefSymbol(BranchUnconditionalImmediate, bi);
    DefSymbol(BranchUnconditionalImmediateLink, bil);
    DefSymbol(BranchUnconditional, b);
    DefSymbol(BranchUnconditionalLink, bl);
    DefSymbol(BranchConditionalTrueImmediate, bit);
    DefSymbol(BranchConditionalTrueImmediateLink, bitl);
    DefSymbol(BranchConditionalTrue, bt);
    DefSymbol(BranchConditionalTrueLink, btl);
    DefSymbol(BranchConditionalFalse, bf);
    DefSymbol(BranchConditionalFalseLink, bfl);
    DefSymbol(BranchConditionalFalseImmediate, bif);
    DefSymbol(BranchConditionalFalseImmediateLink, bifl);
    DefSymbol(IfThenElseTrue, ift);
    DefSymbol(IfThenElseFalse, iff);
    DefSymbol(IfThenElseTrueLink, iftl);
    DefSymbol(IfThenElseFalseLink, iffl);
    DefSymbol(Eq, eq);
    DefSymbol(Neq, ne);
    DefSymbol(Lt, lt);
    DefSymbol(Gt, gt);
    DefSymbol(Le, le);
    DefSymbol(Ge, ge);
    DefSymbol(EqImmediate, eqi);
    DefSymbol(NeqImmediate, nei);
    DefSymbol(LtImmediate, lti);
    DefSymbol(GtImmediate, gti);
    DefSymbol(LeImmediate, lei);
    DefSymbol(GeImmediate, gei);
    DefSymbol(Push, push);
    DefSymbol(PushImmediate, pushi);
    DefSymbol(Pop, pop);
    DefSymbol(MoveToIP, mtip);
    DefSymbol(MoveFromIP, mfip);
    DefSymbol(MoveToLR, mtlr);
    DefSymbol(MoveFromLR, mflr);
    DefSymbol(SaveCRs, svcr);
    DefSymbol(RestoreCRs, recr);
    DefSymbol(CRXor, crxor);
    DefSymbol(CRNot, crnot);
    DefSymbol(CRAnd, crand);
    DefSymbol(CROr, cror);
    DefSymbol(CRNand, crnand);
    DefSymbol(CRNor, crnor);
    DefSymbol(CRSwap, crswap);
    DefSymbol(CRMove, crmove);
    DefSymbol(BranchLR, blr);
    DefSymbol(BranchLRLink, blrl);
    DefSymbol(BranchLRTrue, blrt);
    DefSymbol(BranchLRTrueLink, blrtl);
    DefSymbol(BranchLRFalse, blrf);
    DefSymbol(BranchLRFalseLink, blrfl);
#undef DefSymbol
}

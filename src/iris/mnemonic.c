#include <string.h>
#include "mnemonic.h"

char* mnemonic(ushort value) {
   datum d;
   instruction i;
   d.value = value;
   i.value = d.rest;
   switch(d.group) {
      case InstructionGroupArithmetic:
         return arithmetic_mnemonic(i);
      case InstructionGroupMove:
         return move_mnemonic(i);
      case InstructionGroupJump:
         return jump_mnemonic(i);
      case InstructionGroupCompare:
         return compare_mnemonic(i);
      default:
         error("invalid instruction group provided",
               ErrorInvalidInstructionGroupProvided);
         return "lolurmom"; /* suppress compiler warnings */
   }
}

char* arithmetic_mnemonic(instruction i) {
   switch(i.arithmetic.op) {
      case ArithmeticOpAdd:        return "add";
      case ArithmeticOpSub:        return "sub";
      case ArithmeticOpMul:        return "mul";
      case ArithmeticOpDiv:        return "div";
      case ArithmeticOpRem:        return "rem";
      case ArithmeticOpShiftLeft:  return "shl";
      case ArithmeticOpShiftRight: return "shr";
      case ArithmeticOpBinaryAnd:  return "and";
      case ArithmeticOpBinaryOr:   return "ior";
      case ArithmeticOpBinaryNot:  return "not";
      case ArithmeticOpBinaryXor:  return "xor";
      default:
         error("invalid arithmetic operation",
               ErrorInvalidArithmeticOperation);
         return "UNKNOWN_ARITHMETIC_OPERATOR";
   }
}

char* move_mnemonic(instruction i) {
   switch(i.move.op) {
      case MoveOpRegToReg:       return "move";
      case MoveOpImmediateToReg: return "store";
      case MoveOpRegToAddress:   return "load";
      default:
         error("invalid move operation conditional type",
               ErrorInvalidMoveOperationConditionalType);
         return "UNKNOWN_MOVE_OPERATOR";
   }
}

char* jump_mnemonic(instruction i) {
   switch(i.jump.conditional) {
      case JumpOpUnconditional: return "goto";
      case JumpOpIfTrue:        return "iftrue";
      case JumpOpIfFalse:       return "iffalse";
      case JumpOpIfThenElse:    return "if";
      default:
         error("invalid jump conditional type",
               ErrorInvalidJumpConditionalType);
         return "UNKNOWN JUMP OPERATOR";
   }
}

char* compare_mnemonic(instruction i) {
   char combinebits[5];

   switch(i.compare.combinebits) {
      case CombineBitsOpNil: strcpy(combinebits, "");     break;
      case CombineBitsOpAnd: strcpy(combinebits, "and_"); break;
      case CombineBitsOpOr:  strcpy(combinebits, "or_");  break;
      case CombineBitsOpXor: strcpy(combinebits, "xor_"); break;
      default:
         error("invalid compare combine bits", ErrorInvalidCombineBits);
         return "???";
   }

   switch(i.compare.op) {
      case CompareOpEq:                   return strcat(combinebits, "eq");
      case CompareOpNeq:                  return strcat(combinebits, "ne");
      case CompareOpLessThan:             return strcat(combinebits, "lt");
      case CompareOpGreaterThan:          return strcat(combinebits, "gt");
      case CompareOpLessThanOrEqualTo:    return strcat(combinebits, "le");
      case CompareOpGreaterThanOrEqualTo: return strcat(combinebits, "ge");
      default:
         error("invalid compare operation", ErrorInvalidCompareOperation);
         return "UNKNOWN COMPARE OPERATOR";
   }
}

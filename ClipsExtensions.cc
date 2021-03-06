/**
 * @file
 * implementation of methods described in ClipsExtensions.h
 * @copyright
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


#include "BaseTypes.h"
#include "Base.h"
#include "BaseArithmetic.h"
#include "ClipsExtensions.h"
#include "functional.h"
#include "ExternalAddressWrapper.h"

#include <cstdint>
#include <climits>
#include <sstream>
#include <memory>
#include <map>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

extern "C" {
#include "clips.h"
}

namespace syn {

void 
BinaryNot(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue val;
	if (!UDFFirstArgument(context, INTEGER_BIT, &val)) {
		setBoolean(env, ret, false);
		return;
	}
	ret->integerValue = CreateInteger(env, ~(CVCoerceToInteger(&val)));
}

void
BinaryOr(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue a, b;
	if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
		setBoolean(env, ret, false);
		return;
	} 

	ret->integerValue = CreateInteger(env, CVCoerceToInteger(&a) | CVCoerceToInteger(&b));
}

void
BinaryAnd(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue a, b;
	if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
		setBoolean(env, ret, false);
		return;
	} 
	ret->integerValue = CreateInteger(env, CVCoerceToInteger(&a) & CVCoerceToInteger(&b));
}

void
BinaryXor(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue a, b;
	if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
		setBoolean(env, ret, false);
		return;
	} 
	ret->integerValue = CreateInteger(env, CVCoerceToInteger(&a) ^ CVCoerceToInteger(&b));
}

void
BinaryNor(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue a, b;
	if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
		setBoolean(env, ret, false);
		return;
	} 

	ret->integerValue = CreateInteger(env,  ~(CVCoerceToInteger(&a) | CVCoerceToInteger(&b)));
}

void
BinaryNand(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue a, b;
	if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
		setBoolean(env, ret, false);
		return;
	} 

	ret->integerValue = CreateInteger(env,  ~(CVCoerceToInteger(&a) & CVCoerceToInteger(&b)));
}

void
ShiftLeft(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue value, by;
	if (!UDFFirstArgument(context, INTEGER_BIT, &value)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &by)) {
		setBoolean(env, ret, false);
		return;
	} 

	ret->integerValue = CreateInteger(env,  (CVCoerceToInteger(&value) << CVCoerceToInteger(&by)));
}

void
ShiftRight(Environment* env, UDFContext* context, UDFValue* ret) {
	UDFValue value, by;
	if (!UDFFirstArgument(context, INTEGER_BIT, &value)) {
		setBoolean(env, ret, false);
		return;
	} else if (!UDFNextArgument(context, INTEGER_BIT, &by)) {
		setBoolean(env, ret, false);
		return;
	} 

	ret->integerValue = CreateInteger(env,  (CVCoerceToInteger(&value) >> CVCoerceToInteger(&by)));
}
	void CLIPS_errorMessageGeneric(Environment* env, UDFContext* context, UDFValue* ret, const char* msg) noexcept {
		UDFInvalidArgumentMessage(context, msg);
		setBoolean(env, ret, false);
	}
	void CLIPS_errorMessageGeneric(Environment* env, UDFContext* context, UDFValue* ret, const std::string& msg) noexcept {
		CLIPS_errorMessageGeneric(env, context, ret, msg.c_str());
	}
	void CLIPS_errorOverflowedNumber(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
		CLIPS_errorMessageGeneric(env, context, ret, "number is too large and overflowed");
	}
	void CLIPS_translateBitmask(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
		UDFValue value;
		if (!UDFFirstArgument(context, LEXEME_BITS, &value)) {
			setBoolean(env, ret, false);
		} else {
			std::string str(value.lexemeValue->contents);
			if (boost::starts_with(str, "0m")) {
				str.at(1) = '0';
				auto tmp = strtoul(str.c_str(), NULL, 2);
				if (tmp == ULONG_MAX && errno == ERANGE) {
					CLIPS_errorOverflowedNumber(env, context, ret);
				} else {
					if (tmp > 0xFF) {
						CLIPS_errorMessageGeneric(env, context, ret, "provided number is larger than 8-bits!");
					} else {
						ret->integerValue = CreateInteger(env,  static_cast<int64_t>(static_cast<byte>(tmp)));
					}
				}
			} else {
				CLIPS_errorMessageGeneric(env, context, ret, "Bitmask must start with 0m");
			}
		}
	}
	void CLIPS_errorNumberLargerThan64Bits(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
		CLIPS_errorMessageGeneric(env, context, ret, "provided number is larger than 64-bits!");
	}


	void CLIPS_expandBit(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
		UDFValue number;
		if (!UDFFirstArgument(context, NUMBER_BITS, &number)) {
			setBoolean(env, ret, false);
			return;
		}
		auto value = CVCoerceToInteger(&number);
		ret->integerValue = CreateInteger(env,  int64_t(expandBit(value != 0)));
	}

	void CLIPS_basePrintAddress(Environment* env, const char* logicalName, void* theValue, const char* func, const char* majorType) {
		std::stringstream ss;
		auto* ptr = static_cast<CLIPSExternalAddress*>(theValue);
		ss << "<" << majorType << "-" << func << "-" << std::hex << ((ptr) ? ptr->contents : theValue) << ">";
		auto str = ss.str();
        clips::printRouter(env, logicalName, str);
	}
	//void CLIPS_basePrintAddress_Pointer(Environment* env, const char* logicalName, void* theValue, const char* func) noexcept {
	//	CLIPS_basePrintAddress(env, logicalName, theValue, func, "Pointer");
	//}
	void CLIPS_decodeBits(Environment* env, UDFContext* context, UDFValue* ret) {
		UDFValue value, mask, shift;
		if (!UDFFirstArgument(context, NUMBER_BITS, &value)) {
			setBoolean(env, ret, false);
		} else if (!UDFNextArgument(context, NUMBER_BITS, &mask)) {
			setBoolean(env, ret, false);
		} else if (!UDFNextArgument(context, NUMBER_BITS, &shift)) {
			setBoolean(env, ret, false);
		} else {
			ret->integerValue = CreateInteger(env,  decodeBits<int64_t, int64_t>(CVCoerceToInteger(&value), CVCoerceToInteger(&mask), CVCoerceToInteger(&shift)));
		}
	}
	void CLIPS_encodeBits(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
		UDFValue input, value, mask, shift;
		if (!UDFFirstArgument(context, NUMBER_BITS, &input)) {
			setBoolean(env, ret, false);
		} else if (!UDFNextArgument(context, NUMBER_BITS, &value)) {
			setBoolean(env, ret, false);
		} else if (!UDFNextArgument(context, NUMBER_BITS, &mask)) {
			setBoolean(env, ret, false);
		} else if (!UDFNextArgument(context, NUMBER_BITS, &shift)) {
			setBoolean(env, ret, false);
		} else {
			auto i = CVCoerceToInteger(&input);
			auto v = CVCoerceToInteger(&value);
			auto m = CVCoerceToInteger(&mask);
			auto s = CVCoerceToInteger(&shift);
			ret->integerValue = CreateInteger(env,  encodeBits<int64_t, int64_t>(i, v, m, s));
		}
	}
    template<typename I, typename O, I mask, int index>
    O performDecode(I input) noexcept {
        return syn::decodeBits<I, O, mask << (8 * index), (8 * index)>(input);
    }
	void CLIPS_breakApartNumber(Environment* env, UDFContext* context, UDFValue* ret) {
		UDFValue number;
		if (!UDFFirstArgument(context, NUMBER_BITS, &number)) {
			setBoolean(env, ret, false);
            return;
		}
        auto integer = CVCoerceToInteger(&number);
        constexpr int integerWidth = byteCount<decltype(integer)>;
        constexpr auto baseMask = static_cast<decltype(integer)>(0xFF);
		maya::MultifieldBuilder mb(env);
        if (integerWidth == 8) {
			mb.append(performDecode<int64_t, int64_t, baseMask, 0>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 1>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 2>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 3>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 4>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 5>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 6>(integer));
			mb.append(performDecode<int64_t, int64_t, baseMask, 7>(integer));
        } else {
            for (int i = 0; i < integerWidth; ++i) {
				mb.append(int64_t(syn::decodeBits<int64_t, int64_t>(integer, baseMask << (8 * i), (8 * i))));
            }
        }
		ret->multifieldValue = mb.create();
	}

	bool errorMessage(Environment* env, const std::string& idClass, int idIndex, const std::string& msgPrefix, const std::string& msg) noexcept {
		PrintErrorID(env, idClass.c_str(), idIndex, false);
        clips::printRouter(env, STDERR, msgPrefix);
        clips::printRouter(env, STDERR, msg);
        clips::printLine(env, STDERR);
		SetEvaluationError(env, true);
		return false;
	}
    template<bool shiftLeft>
    void CLIPS_circularShiftBase(Environment* env, UDFContext* context, UDFValue* ret) {
        UDFValue a, b;
        if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
            setBoolean(env, ret, false);
        } else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
            setBoolean(env, ret, false);
        } else {
            auto firstValue = CVCoerceToInteger(&a);
            auto secondValue = CVCoerceToInteger(&b);
            auto result = shiftLeft ?
                circularShiftLeft<int64_t>(firstValue, secondValue) :
                circularShiftRight<int64_t>(firstValue, secondValue);
            ret->integerValue = CreateInteger(env,  result);
        }
    }
    void CLIPS_circularShiftLeft(Environment* env, UDFContext* context, UDFValue* ret) {
        CLIPS_circularShiftBase<true>(env, context, ret);
    }
    void CLIPS_circularShiftRight(Environment* env, UDFContext* context, UDFValue* ret) {
        CLIPS_circularShiftBase<false>(env, context, ret);
    }
    void CLIPS_onesComplement(Environment* env, UDFContext* context, UDFValue* ret) {
        UDFValue val;
        if (!UDFFirstArgument(context, INTEGER_BIT, &val)) {
            setBoolean(env, ret, false);
        } else {
            ret->integerValue = CreateInteger(env,  onesComplement(CVCoerceToInteger(&val)));
        }
    }
    void CLIPS_twosComplement(Environment* env, UDFContext* context, UDFValue* ret) {
        UDFValue val;
        if (!UDFFirstArgument(context, INTEGER_BIT, &val)) {
            setBoolean(env, ret, false);
        } else {
            ret->integerValue = CreateInteger(env,  twosComplement(CVCoerceToInteger(&val)));
        }
    }
    void CLIPS_multiplyAdd(Environment* env, UDFContext* context, UDFValue* ret) {
        UDFValue a, b, c;
        if (!UDFFirstArgument(context, INTEGER_BIT, &a)) {
            setBoolean(env, ret, false);
        } else if (!UDFNextArgument(context, INTEGER_BIT, &b)) {
            setBoolean(env, ret, false);
        } else if (!UDFNextArgument(context, INTEGER_BIT, &c)) {
            setBoolean(env, ret, false);
        } else {
            ret->integerValue = CreateInteger(env,  multiplyAdd(CVCoerceToInteger(&a), CVCoerceToInteger(&b), CVCoerceToInteger(&c)));
        }
    }
    void CLIPS_getEndianness(Environment* env, UDFContext* context, UDFValue* ret) {
        static bool init = true;
        static std::string storage;
        if (init) {
            init = false;
            if (syn::isBigEndian()) {
                storage = "big";
            } else if (syn::isLittleEndian()) {
                storage = "little";
            } else {
                storage = "unknown";
            }
        }
        // only compute this once!
		ret->lexemeValue = CreateSymbol(env, storage.c_str());
    }

    template<bool upperHalf>
    void upperLowerHalfManip(Environment* env, UDFContext* context, UDFValue* ret) {
        UDFValue num;
        if (!UDFFirstArgument(context, INTEGER_BIT, &num)) {
            setBoolean(env, ret, false);
        } else {
            int64_t n = num.integerValue->contents;
            if (upperHalf) {
				ret->integerValue = CreateInteger(env, syn::decodeBits<int64_t, int64_t, static_cast<int64_t>(0xFFFFFFFF00000000), getShiftCount<int64_t>()>(n));
            } else {
				ret->integerValue = CreateInteger(env, decodeBits<int64_t, int64_t, 0x00000000FFFFFFFF, 0>(n));
            }
        }
    }

    void CLIPS_getUpperHalf(Environment* env, UDFContext* c, UDFValue* ret) { upperLowerHalfManip<true>(env, c, ret); }
    void CLIPS_getLowerHalf(Environment* env, UDFContext* c, UDFValue* ret) { upperLowerHalfManip<false>(env, c, ret); }

	void CLIPS_translateBinary(Environment* env, UDFContext* context, UDFValue* ret) noexcept;
	void CLIPS_translateHex(Environment* env, UDFContext* context, UDFValue* ret) noexcept;
	void CLIPS_translateOctal(Environment* env, UDFContext* context, UDFValue* ret) noexcept;

	void installExtensions(Environment* theEnv) {

		AddUDF(theEnv, "bitmask->int", "l", 1, 1, "sy", CLIPS_translateBitmask, "CLIPS_translateBitmask", nullptr);
		AddUDF(theEnv, "expand-bit", "l", 1, 1,  nullptr,  CLIPS_expandBit, "CLIPS_expandBit",  nullptr);
		AddUDF(theEnv, "decode-bits", "l", 3, 3, "l;l;l",   CLIPS_decodeBits, "CLIPS_decodeBits",nullptr);
		AddUDF(theEnv, "encode-bits", "l", 4, 4, "l;l;l;l", CLIPS_encodeBits, "CLIPS_encodeBits",nullptr);
		AddUDF(theEnv, "break-apart-number", "m",    1, 1, "l",   CLIPS_breakApartNumber, "CLIPS_breakApartNumber",    nullptr);
        AddUDF(theEnv, "circular-shift-right", "l",  2, 2, "l;l", CLIPS_circularShiftRight, "CLIPS_circularShiftRight",nullptr);
        AddUDF(theEnv, "circular-shift-left", "l",   2, 2, "l;l", CLIPS_circularShiftLeft, "CLIPS_circularShiftLeft",  nullptr);
        AddUDF(theEnv, "ones-complement", "l",  1, 1, "l",     CLIPS_onesComplement, "CLIPS_onesComplement",nullptr);
        AddUDF(theEnv, "twos-complement", "l",  1, 1, "l",     CLIPS_twosComplement, "CLIPS_twosComplement",nullptr);
        AddUDF(theEnv, "multiply-add", "l",     3, 3, "l;l;l", CLIPS_multiplyAdd, "CLIPS_multiplyAdd",      nullptr);
        AddUDF(theEnv, "get-endian", "sy",      0, 0, nullptr, CLIPS_getEndianness, "CLIPS_getEndianness",  nullptr);
        AddUDF(theEnv, "upper-half", "l",  1, 1, "l", CLIPS_getUpperHalf, "CLIPS_getUpperHalf",nullptr);
        AddUDF(theEnv, "lower-half", "l",  1, 1, "l", CLIPS_getLowerHalf, "CLIPS_getLowerHalf",nullptr);
		AddUDF(theEnv, "binary-not", "l",  1, 1, "l",   BinaryNot, "BinaryNot",   nullptr);
		AddUDF(theEnv, "binary-and", "l",  2, 2, "l;l", BinaryAnd, "BinaryAnd",   nullptr);
		AddUDF(theEnv, "binary-or", "l",   2, 2, "l;l", BinaryOr, "BinaryOr",     nullptr);
		AddUDF(theEnv, "binary-xor", "l",  2, 2, "l;l", BinaryXor, "BinaryXor",   nullptr);
		AddUDF(theEnv, "binary-nand", "l", 2, 2, "l;l", BinaryNand, "BinaryNand", nullptr);
		AddUDF(theEnv, "binary-nor", "l",  2, 2, "l;l", BinaryNor, "BinaryNor",   nullptr);
		AddUDF(theEnv, "left-shift", "l",   2, 2, "l;l",ShiftLeft, "ShiftLeft",    nullptr);
		AddUDF(theEnv, "right-shift", "l", 2, 2, "l;l", ShiftRight, "ShiftRight", nullptr);
		AddUDF(theEnv, "binary->int", "l", 1, 1, "sy", CLIPS_translateBinary, "CLIPS_translateBinary", nullptr);
		AddUDF(theEnv, "hex->int", "l", 1, 1, "sy", CLIPS_translateHex, "CLIPS_translateHex", nullptr);
		AddUDF(theEnv, "oct->int", "l", 1, 1, "sy", CLIPS_translateOctal, "CLIPS_translateOctal", nullptr);
	}

    void buildFunctionString(std::ostream& stream, const std::string& action, const std::string& name) noexcept {
        stream << "Function " << action << " (" << name << ")";
    }
    void buildFunctionErrorString(std::ostream& stream, const std::string& action, const std::string& name) noexcept {
        stream << "Function ";
        buildFunctionString(stream, action, name);
	}

void CLIPS_translateNumberBase(Environment* env, UDFContext* context, UDFValue* ret, const std::string& prefix, int base, const std::string& badPrefix, bool zeroPositionOne = false) noexcept {
	constexpr uint64_t maximumIntegerValue = 0xFFFFFFFFFFFFFFFF;
	UDFValue value;
	if (!UDFFirstArgument(context, LEXEME_BITS, &value)) {
		setBoolean(context, ret, false);
	} 
	std::string str(getLexeme(value));
	if (boost::starts_with(str, prefix)) {
		if (zeroPositionOne) {
			str.at(1) = '0';
		}
		auto tmp = strtoull(str.c_str(), nullptr, base);
		if (tmp == ULLONG_MAX && errno == ERANGE) {
			CLIPS_errorOverflowedNumber(env, context, ret);
		} else {
			if (tmp > maximumIntegerValue) {
				CLIPS_errorNumberLargerThan64Bits(env, context, ret);
			} else {
				setInteger(env, ret, tmp);
			}
		}
	} else {
		CLIPS_errorMessageGeneric(env, context, ret, badPrefix);
	}
}
void CLIPS_translateBinary(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
	CLIPS_translateNumberBase(env, context, ret, "0b", 2, "Binary must start with 0b", true);
}

void CLIPS_translateHex(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
	CLIPS_translateNumberBase(env, context, ret, "0x", 16, "Hex must start with 0x");
}

void CLIPS_translateOctal(Environment* env, UDFContext* context, UDFValue* ret) noexcept {
	CLIPS_translateNumberBase(env, context, ret, "0q", 8, "Octal must start with 0q", true);
}

}

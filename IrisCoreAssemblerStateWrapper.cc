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


// IrisCoreAssembler rewritten to use pegtl
#include <sstream>
#include <typeinfo>
#include <iostream>
#include <map>
#include "Base.h"
#include "AssemblerBase.h"
#include "Problem.h"
#include "IrisCore.h"
#include <pegtl.hh>
#include <pegtl/analyze.hh>
#include <pegtl/file_parser.hh>
#include <pegtl/contrib/raw_string.hh>
#include <pegtl/contrib/abnf.hh>
#include <pegtl/parse.hh>
#include <vector>
#include "IrisClipsExtensions.h"
#include "ClipsExtensions.h"
#include "IrisCoreAssembler.h"
#include "IrisCoreAssemblerStateWrapper.h"

namespace iris {
    class AssemblerState;
}
namespace syn {
    DefWrapperSymbolicName(iris::AssemblerState, "iris:assembly-parsing-state");
}
namespace iris {


    void AssemblerStateWrapper::getMultifield(void* env, CLIPSValuePtr ret) {
        //get()->output(env, ret);
        output(env, ret);
    }
    bool AssemblerStateWrapper::resolve() {
        auto & state = *(get());
		auto resolveLabel = [&state](AssemblerData& data) {
			auto result = state.findLabel(data.currentLexeme);
			if (result == state.labelsEnd()) {
				std::stringstream msg;
				msg << "ERROR: label " << data.currentLexeme << " is undefined!" << std::endl;
				auto str = msg.str();
				throw syn::Problem(str);
			} else {
				return result->second;
			}
		};
        state.applyToFinishedData([resolveLabel](auto value) {
                    if (value.shouldResolveLabel()) {
                        auto result = resolveLabel(value);
                        if (value.instruction) {
                            value.setImmediate(result);
                        } else {
                            value.dataValue = result;
                        }
                    }
                });
        return true;
    }
    bool AssemblerStateWrapper::parseLine(const std::string& line) {
        auto& ref = *(get());
        return pegtl::parse_string<iris::Main, iris::Action>(line, "clips-input", ref);
    }
    void AssemblerStateWrapper::output(void* env, CLIPSValue* ret) noexcept {
        // we need to build a multifield out of the finalWords
        auto & state = *(get());
        syn::MultifieldBuilder f(env, state.numberOfFinishedItems() * 3);
        auto body = [&f, env, ret](auto value, auto baseIndex) noexcept {
            auto i = (3 * baseIndex) + 1;
            f.setField(i + 0, INTEGER, EnvAddLong(env, value.instruction ? 0 : 1));
            f.setField(i + 1, INTEGER, EnvAddLong(env, value.address));
            f.setField(i + 2, INTEGER, EnvAddLong(env, value.encode()));
        };
        state.applyToFinishedData(body);
        f.assign(ret);
    }

    void installAssemblerParsingState(void* env) {
        pegtl::analyze<iris::Main>();
        AssemblerStateWrapper::registerWithEnvironment(env);
        AssemblerStateWrapper::registerWithEnvironment(env, "iris-asm-parser");
        AssemblerStateWrapper::registerWithEnvironment(env, "iris-assembler");
    }
}

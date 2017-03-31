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


// register all of the machine cores here since the cores should not be aware
// of these kinds of registrations
#include "Problem.h"
#include "RegisterEntry.h"
#include "AssemblerRegistrar.h"
#include "IrisCore.h"
#include "Cisc0Core.h"

template<typename T>
void assemble(const std::string& file, FILE* ptr, std::ostream* output) {
    T::assemble(file, ptr, output);
}
template<typename T>
class RegisterAssembler : public syn::RegisterEntry<syn::AssemblerRegistrar, T> {
    public:
        using Self = RegisterAssembler<T>;
        using Parent = syn::RegisterEntry<syn::AssemblerRegistrar, T>;
        using Operation = syn::AssemblerRegistrar::Operation;
    public:
        RegisterAssembler(syn::AssemblerRegistrar& reg, const std::string& name, Operation op) : Parent(reg, name, op) { }
        RegisterAssembler(syn::AssemblerRegistrar& reg, const std::string& name) : Self(reg, name, assemble<T>) { }
};

static RegisterAssembler<iris::Core> iris16Core(syn::assemblerRegistry, "iris");
static RegisterAssembler<cisc0::Core> cisc0Core(syn::assemblerRegistry, "cisc0");

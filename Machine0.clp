;------------------------------------------------------------------------------
; syn
; Copyright (c) 2013-2017, Joshua Scoggins and Contributors
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;------------------------------------------------------------------------------
; Machine0.clp - First architecture using the new ucoded techniques
;------------------------------------------------------------------------------
; The machine0 core is a 64-bit signed integer architecture with a 64-bit word
; and a 27-bit memory space. If the addresses are expanded to the byte level then it would
; be a 30-bit memory space!
; This 27 bit memory space is divided up into 8 16 mega word sections!
(batch* cortex.clp)
(batch* MainModuleDeclaration.clp)
(batch* ExternalAddressWrapper.clp)
(batch* Device.clp)
(batch* MemoryBlock.clp)
(batch* Register.clp)
(batch* order.clp)
(defclass MAIN::machine0-memory-block
  (is-a memory-block)
  (slot capacity
        (source composite)
        (storage shared)
        (default (+ (hex->int 0x00FFFFFF) 
                    1))))
(defclass MAIN::register-file
          (is-a memory-block)
          (slot capacity
                (source composite)
                (storage shared)
                (default 16)))
; There are 8 memory spaces in this machine setup for a total of 1 gigabyte or 128 megawords
(definstances MAIN::machine0-memory-spaces
              (space0 of machine0-memory-block)
              ;(space1 of machine0-memory-block)
              ;(space3 of machine0-memory-block)
              ;(space3 of machine0-memory-block)
              ;(space4 of machine0-memory-block)
              ;(space5 of machine0-memory-block)
              ;(space6 of machine0-memory-block)
              ;(space7 of machine0-memory-block)
              )

; The instruction pointer register is 27-bits wide or having a mask of 0x07FFFFFF 
; this applies to the stack register as well. All bits above the mask must be zero to maintain
; backwards compatibility
(defglobal MAIN
           ?*address-mask* = (hex->int 0x00FFFFFF)
           ?*address-mask27* = (hex->int 0x07FFFFFF))
(definstances MAIN::machine0-registers
              (rf0 of register-file)
              (ip of register
                  (mask ?*address-mask*))
              (sp of register
                  (mask ?*address-mask*)))


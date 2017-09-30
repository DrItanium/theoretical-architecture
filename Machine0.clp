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
(deffacts MAIN::make-memory-blocks
          (make memory-block named space0)
          ;(make memory-block named space1)
          ;(make memory-block named space2)
          ;(make memory-block named space3)
          ;(make memory-block named space4)
          ;(make memory-block named space5)
          ;(make memory-block named space6)
          ;(make memory-block named space7)
          )

; The instruction pointer register is 27-bits wide or having a mask of 0x07FFFFFF 
; this applies to the stack register as well. All bits above the mask must be zero to maintain
; backwards compatibility
(defglobal MAIN
           ?*address-mask* = (hex->int 0x00FFFFFF)
           ?*address-mask27* = (hex->int 0x07FFFFFF))
; Internally, machine
(deffacts MAIN::make-registers
          ;(terminate at 128 cycles)
          ;(terminate at ?*address-mask* cycles)
          (terminate at (hex->int 0xFFFF) cycles)
          (make register named ip with mask ?*address-mask*)
          (make register named sp with mask ?*address-mask*)
          (make register named cs with mask ?*address-mask*)
          (make register named dictionary with mask ?*address-mask*)
          (make register named t0)
          (make register named t1)
          (make register named t2))

(defglobal MAIN
           ?*execution-cycle-stages* = (create$ read ; load the instruction from memory
                                                eval ; evaluate instruction input and construct the ucode sequence
                                                print ; invoke the execution unit and save the result as needed
                                                advance ; advance ip
                                                loop ; perform checks to see if we should loop or terminate
                                                ))
(deffacts MAIN::cycles
          (stage (current startup)
                 (rest initialize
                       execute
                       ?*execution-cycle-stages*
                       shutdown)))
; the cpu bootstrap process requires lower priority because it always exists in the background and dispatches 
; cycles as we go along. This is how we service interrupts and other such things. 
(defrule MAIN::boostrap-startup
         (declare (salience ?*priority:first*)) 
         (stage (current startup))
         =>
         (printout t 
                   "Machine0 System boot" crlf
                   "Starting up .... please wait" crlf))

(defrule MAIN::initialize:describe-phase
         (declare (salience ?*priority:first*))
         (stage (current initialize))
         =>
         (printout t
                   "Initializing memory space ... please wait" crlf))

(defrule MAIN::initialize:make-memory-block
         (stage (current initialize))
         ?f <- (make memory-block named ?name)
         =>
         (retract ?f)
         (printout t 
                   "Bringing up memory block: " ?name " .... ")
         (make-instance ?name of machine0-memory-block)
         (printout t 
                   Done crlf
                   tab (format nil
                               "size: 0x%x words"
                               (send (symbol-to-instance-name ?name)
                                     size)) crlf))
(defrule MAIN::initialize:make-register-file
         (stage (current initialize))
         ?f <- (make register-file named ?name)
         =>
         (retract ?f)
         (printout t
                   "Bringing up register-file: " ?name " .... ")
         (make-instance ?name of register-file)
         (printout t
                   Done crlf
                   tab (format nil
                               "register count: %d"
                               (send (symbol-to-instance-name ?name)
                                     size)) crlf))
(defrule MAIN::initialize:make-register-with-mask
         (stage (current initialize))
         ?f <- (make register named ?name with mask ?mask)
         =>
         (retract ?f)
         (printout t
                   "Bringing up register: " ?name " .... ")
         (make-instance ?name of register 
                        (mask ?mask))
         (printout t 
                   Done crlf
                   tab (format nil 
                               "mask: 0x%x"
                               ?mask) crlf))
(defrule MAIN::initialize:make-register-default
         (stage (current initialize))
         ?f <- (make register named ?name)
         =>
         (retract ?f)
         (printout t
                   "Bringing up register: " ?name " .... ")
         (make-instance ?name of register)
         (printout t
                   Done crlf))

(defrule MAIN::shutdown:print-phase
         (declare (salience ?*priority:first*))
         (stage (current shutdown))
         =>
         (printout t
                   "Shutting down machine0 system!" crlf))
(defrule MAIN::shutdown:delete-memory-block
         (stage (current shutdown))
         (object (is-a machine0-memory-block)
                 (name ?name))
         =>
         (assert (delete "memory block" 
                         ?name)))

(defrule MAIN::shutdown:delete-register-file
         (stage (current shutdown))
         (object (is-a register-file)
                 (name ?name))
         =>
         (assert (delete "register file"
                         ?name)))

(defrule MAIN::shutdown:delete-register
         (stage (current shutdown))
         (object (is-a register)
                 (name ?name))
         =>
         (assert (delete register ?name)))
(defrule MAIN::shutdown:delete-thingy
         (stage (current shutdown))
         ?f <- (delete ?title ?name)
         =>
         (retract ?f)
         (printout t
                   "Bringing down " ?title ": " (instance-name-to-symbol ?name) " .... ")
         (unmake-instance ?name)
         (printout t
                   Done crlf))

(defrule MAIN::shutdown:shutdown-complete
         (declare (salience -9000))
         (stage (current shutdown))
         =>
         (printout t
                   "shutdown complete .... bye" crlf))


(defrule MAIN::execute:generate-cycle-execute
         "setup the execution cycle!"
         (stage (current execute))
         =>
         (printout t
                   "Setting up the execution cycle!" crlf))

; The layout of the instruction is pretty simple, if the number is negative then it is
; a branch instruction
(deffunction MAIN::branch-instructionp
             "The most significant bit of an instruction signifies if it is a branch instruction or not"
             (?value)
             ; in this case it is bit 63,
             ; the proper code is:
             ; (<> (decode-bits ?value
             ;                 (hex->int 0x8000000000000000)
             ;                 62)
             ;    0))
             ; but since CLIPSIntegers are signed then we can just imply twos compliment
             (< ?value 
                0))

; The next 7 bits (56-62) are the group bits, the upper most 8 bits have the same purpose in 
; jumps and everything else!
(deffunction MAIN::get-group-bits
             (?value)
             (decode-bits ?value
                          (hex->int 0x7F00000000000000)
                          56))

(deffunction MAIN::instruction-volatile-bits
             (?value)
             (decode-bits ?value
                          (hex->int 0x00FFFFFFFFFFFFFF)
                          0))

(defrule MAIN::execute:execution-cycle:read-from-memory
         (stage (current read))
         (object (is-a register)
                 (name [ip])
                 (value ?addr))
         ?ms0 <- (object (is-a machine0-memory-block)
                         (name [space0]))
         =>
         (assert (instruction ?addr
                              (bind ?value
                                    (send ?ms0
                                          read
                                          ?addr)))))

(defrule MAIN::eval:delete-nops
         "End the cycle early since this is a nop so discard the operation"
         (declare (salience ?*priority:first*))
         (stage (current eval))
         ?f <- (instruction ?addr
                            0)
         =>
         ; just retract the nop instruction as it does nothing!
         (retract ?f))


; When dealing with non branch instructions, we have further bits defined for operations, the next
; 8 bits define the operation category
(deffunction MAIN::extract-operation-field
             (?value)
             (decode-bits ?value
                          (hex->int 0x00FF000000000000)
                          48))

(deftemplate MAIN::operation
             (slot address
                   (type INTEGER)
                   (default ?NONE))
             (slot original-value
                   (type INTEGER)
                   (default ?NONE))
; use a system of objects that are fixed in concept to describe the different fields that
; make up the instruction. 
(defclass MAIN::primary-class-descriptor
          "Is the instruction a branch or non-branch instruction?"
          (is-a thing)
          (slot parent
                (source composite)
                (default FALSE))
          (slot matches-with
                (type SYMBOL)
                (allowed-symbols FALSE
                                 TRUE)
                (storage local)
                (visibility public)
                (access initialize-only)
                (default ?NONE))
          (slot title
                (type SYMBOL)
                (allowed-symbols branch
                                 non-branch)
                (storage local)
                (visibility public)
                (access initialize-only)
                (default ?NONE)))

(definstances MAIN::primary-class-descriptors
              ([primary-class:branch] of primary-class-descriptor
                                      (matches-with TRUE)
                                      (title branch))
              ([primary-class:non-branch] of primary-class-descriptor
                                          (matches-with FALSE)
                                          (title non-branch)))
(defclass MAIN::bits-descriptor
          (is-a thing)
          (slot matches-with
                (type INTEGER)
                (range 0 127)
                (storage local)
                (visibility public)
                (access initialize-only)
                (default ?NONE))
          (slot title
                (type SYMBOL)
                (storage local)
                (visibility public)
                (access initialize-only)
                (default ?NONE)))

(defclass MAIN::group-bits-descriptor
          (is-a bits-descriptor)
          (slot parent
                (source composite)
                (allowed-classes primary-class-descriptor))
          (slot matches-with
                (source composite)
                (range 0 127)))

(defclass MAIN::branch-group-bits-descriptor
          (is-a groups-bits-descriptor)
          (slot parent
                (source composite)
                (default [primary-class:branch])))

(definstances MAIN::branch-group-ops
              ([group-branch:immediate-direct] of branch-group-bits-descriptor
                                        (matches-with 0)
                                        (title immediate))
              ([group-branch:immediate-indirect] of branch-group-bits-descriptor
                                                 (matches-with 1)
                                                 (title immediate-indirect))
              ([group-branch:register] of branch-group-bits-descriptor
                                    (matches-with 2)
                                    (title register))
              ([group-branch:register-indirect] of branch-group-bits-descriptor
                                    (matches-with 3)
                                    (title register-indirect))
              ([group-branch:register-computed] of branch-group-bits-descriptor
                                                (matches-with 4)
                                                (title register-computed)))

(defclass MAIN::operation-bits-descriptor
          (is-a bits-descriptor)
          (slot parent
                (source composite)
                (allowed-classes group-bits-descriptor))
          (slot matches-with
                (source composite)
                (range 0 255)))

(defrule MAIN::eval:get-more-information
         "Not a nop instruction at all, so we need to decode it further"
         (stage (current eval))
         ?f <- (instruction ?addr
                            ?value)
         =>
         (retract ?f)
         (assert (operation (address ?addr)
                            (value ?value))))

; These are constant fields that we know about so we should keep them around
(defrule MAIN::eval:mark-branch-target
         (declare (salience ?*priority:first*))
         (stage (current eval))
         ?f <- (operation (original-value ?value)
                          (chain $?chain))
         (object (is-a primary-class-descriptor)
                 (matches-with =(branch-instructionp ?value))
                 (name ?descriptor))
         =>
         (modify ?f
                 (chain $?chain 
                        ?descriptor)))
(defrule MAIN::eval:mark-group-bits
         (declare (salience ?*priority:first*))
         (stage (current eval))
         ?f <- (operation (group UNKNOWN)
                          (original-value ?value))
         =>
         (modify ?f
                 (group (get-group-bits ?value))))

(defrule MAIN::eval:mark-operation
         

(defrule MAIN::execute:execution-cycle:advance:next-address
         "If we didn't update the instruction pointer then make sure we do that now!"
         (stage (current advance))
         (not (check [ip]))
         ?ip <- (object (is-a register)
                        (name [ip]))
         =>
         (assert (check [ip]))
         (send ?ip 
               increment))

(defrule MAIN::loop:restart-cycle
         ?f <- (stage (current loop)
                      (rest $?rest))
         ?f2 <- (check ?ip)
         (object (is-a register)
                 (name ?ip)
                 (value ?value))
         (terminate at ?addr cycles)
         (test (<> ?value 
                   ?addr))
         =>
         (retract ?f2)
         (modify ?f 
                 (rest ?*execution-cycle-stages*
                       $?rest)))



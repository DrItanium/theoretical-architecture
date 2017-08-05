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
(defmodule test
           "Testing related operations"
           (import cortex 
                   ?ALL)
           (export ?ALL))
(defgeneric test::check-result
            "Check and see if the given result is expected")

(defmethod test::check-result
           ((?router SYMBOL)
            ?expected
            ?value)
           (if (neq ?expected
                    ?value) then

               (printout ?router 
                         "CHECK FAILED: expected " ?expected " but got " ?value crlf)
               FALSE
               else
               TRUE))

(defgeneric test::display-testcase)

(defmethod test::display-testcase
           ((?router SYMBOL)
            (?id INTEGER
                 LEXEME)
            (?title LEXEME))
           (printout ?router
                     "Testcase " ?id ": " ?title crlf))

(defgeneric test::testcase)

(defmethod test::testcase
           ((?router SYMBOL)
            (?id INTEGER
                 LEXEME)
            (?title LEXEME)
            (?function SYMBOL)
            $?arguments)
           (display-testcase ?router
                             ?id
                             ?title)
           (bind ?result
                 (funcall ?function
                  $?arguments))
           (printout ?router
                     tab "Result: " 
                     (if ?result then
                       PASSED
                       else
                       (halt)
                       FAILED) crlf))
               

            


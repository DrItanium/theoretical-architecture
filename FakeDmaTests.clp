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
(deffunction fake-dma-test
             "seed memory with random numbers"
             (?seed ?size ?rng ?device ?fdev)
             ; first seed the rng
             (write-command ?rng
                            (format nil
                                    "seed %d"
                                    ?seed))
             ; skip the first three
             (write-command ?rng skip)
             (write-command ?rng skip)
             (write-command ?rng skip)
             ; start at address zero
             (bind ?read-rng
                   (str-cat "read to " ?fdev))

             (loop-for-count (?i 0 ?size) do
                             (write-command ?rng
                                            ?read-rng)
                             (bind ?k 
                                   (explode$ (read-command)))
                             (bind ?cmd
                                   (str-cat "write " 
                                            ?i
                                            " "
                                            (nth$ 1
                                                  ?k)
                                            " callback "
                                            ?fdev))
                             (write-command ?device
                                            ?cmd)
                             ; ditch the result
                             (read-command)
                             ; skip two random numbers
                             (write-command ?rng
                                            skip)
                             (write-command ?rng
                                            skip)))

(deffunction fake-dma-test2
             "seed memory with random numbers from local"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (loop-for-count (?i 0 ?size) do
                             (bind ?k
                                   (integer (random)))
                             (bind ?cmd
                                   (format nil
                                           "write %d %d callback %s"
                                           ?i
                                           (integer (random))
                                           ?fdev))
                             (write-command ?device
                                            ?cmd)
                             ; ditch the result
                             (read-command)
                             ; skip two random numbers
                             (random)
                             (random)))

(deffunction fake-dma-test2
             "seed memory with random numbers from local"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (loop-for-count (?i 0 ?size) do
                             (write-command ?device
                                            (format nil
                                                    "write %d %d callback %s"
                                                    ?i
                                                    (integer (random))
                                                    ?fdev))
                             ; ditch the result
                             (read-command)
                             ; skip two random numbers
                             (random)
                             (random)))

(deffunction fake-dma-test3
             "seed memory with random numbers from local writing multiple entries at a time"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           ?i
                                           (integer (random))
                                           ?fdev))
                    ; ditch the result
                    ; skip two random numbers
                    (random)
                    (random)
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           (+ ?i 1)
                                           (integer (random))
                                           ?fdev))
                    (random)
                    (random)
                    (bind ?i
                          (+ ?i 2))
                    (read-command)
                    (read-command)))

(deffunction fake-dma-test4
             "seed memory with random numbers from local writing multiple entries at a time (4)"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           ?i
                                           (integer (random))
                                           ?fdev))
                    ; ditch the result
                    ; skip two random numbers
                    (random)
                    (random)
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           (+ ?i 1)
                                           (integer (random))
                                           ?fdev))
                    (random)
                    (random)
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           (+ ?i 2)
                                           (integer (random))
                                           ?fdev))
                    (random)
                    (random)
                    (write-command ?device
                                   (format nil
                                           "write %d %d callback %s"
                                           (+ ?i 3)
                                           (integer (random))
                                           ?fdev))
                    (random)
                    (random)
                    (bind ?i
                          (+ ?i 4))
                    (read-command)
                    (read-command)
                    (read-command)
                    (read-command)))
(deffunction MAIN::set-write
             (?callback $?values)
             (format nil
                     "write set: %s callback %s"
                     (implode$ ?values)
                     ?callback))
(deffunction MAIN::map-write
             (?callback ?base-address $?values)
             (format nil
                     "write map: %d %s callback %s"
                     ?base-address
                     (implode$ ?values)
                     ?callback))

(deffunction fake-dma-test5
             "seed memory with random numbers from local writing multiple entries at a time compacted into one message"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (write-command ?device
                                   (set-write ?fdev
                                              ?i (irandom)
                                              (+ ?i 1) (irandom)
                                              (+ ?i 2) (irandom)
                                              (+ ?i 3) (irandom)))

                    ; skip multiple random numbers
                    (random)
                    (random)
                    (random)
                    (random)
                    (random)
                    (random)
                    (random)
                    (random)
                    (bind ?i
                          (+ ?i 4))
                    (read-command)
                    (read-command)
                    (read-command)
                    (read-command)))
(deffunction fake-dma-test6
             "seed memory with random numbers from local writing multiple entries at a time compacted into one message"
             (?seed ?size ?device ?fdev)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (write-command ?device
                                   (set-write ?fdev
                                              ?i (irandom)
                                              (+ ?i 1) (irandom)
                                              (+ ?i 2) (irandom)
                                              (+ ?i 3) (irandom)
                                              (+ ?i 4) (irandom)
                                              (+ ?i 5) (irandom)
                                              (+ ?i 6) (irandom)
                                              (+ ?i 7) (irandom)))

                    ; skip multiple random numbers
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (random) (random)
                    (bind ?i
                          (+ ?i 8))
                    (read-command) (read-command) (read-command) (read-command)
                    (read-command) (read-command) (read-command) (read-command)
                    ))

(deffunction fake-dma-test7
             "seed memory with random numbers from local writing multiple entries at a time compacted into one message, tweakable"
             (?seed ?size ?device ?fdev ?message-size)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (bind ?contents
                          (create$))
                    (loop-for-count (?j 0 (- ?message-size 1)) do
                                    (bind ?contents
                                          ?contents
                                          (+ ?i ?j)
                                          (irandom))
                                    (random)
                                    (random))
                    (write-command ?device
                                   (set-write ?fdev
                                              ?contents))

                    ; skip multiple random numbers
                    (bind ?i
                          (+ ?i 
                             ?message-size))
                    (loop-for-count (?j 1 ?message-size) do (read-command))))

(deffunction vliw-sendoff
             (?size ?outdev ?callback)
             (timer (fake-dma-test7 (irandom) 
                                    (integer (- (** 2 24) 1)) 
                                    ?outdev
                                    ?callback
                                    ?size)))

(deffunction fake-dma-test8
             "seed memory with random numbers from local writing multiple entries at a time compacted into one message, tweakable and using the map command instead of set"
             (?seed ?size ?device ?fdev ?message-size)
             ; first seed the rng
             (seed ?seed)
             ; skip the first three
             (random)
             (random)
             (random)
             ; start at address zero
             (bind ?i 
                   0)
             (while (< ?i ?size) do
                    (bind ?contents
                          (create$))
                    (loop-for-count (?j 1 ?message-size) do
                                    (bind ?contents
                                          ?contents
                                          (irandom))
                                    (random)
                                    (random))
                    (write-command ?device
                                   (map-write ?fdev
                                              ?i
                                              ?contents))

                    ; skip multiple random numbers
                    (bind ?i
                          (+ ?i 
                             ?message-size))
                    (read-command)))
(deffunction vliw-map-test
             (?size ?outdev ?callback)
             (timer (fake-dma-test8 (irandom)
                                    (integer (- (** 2 24) 1))
                                    ?outdev
                                    ?callback
                                    ?size)))

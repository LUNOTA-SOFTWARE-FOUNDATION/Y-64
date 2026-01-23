;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

mov g1, 0x01    ;; Select IntConf sreg
srr             ;; Read it -> G0
or g0, 1        ;; Mask IRQs
srw             ;; Write it back

mov g0, 0x3C    ;; ITR base
litr g0         ;; Load it
hlt             ;; Halt

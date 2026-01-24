;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

_start:
    mov g0, 0x110000    ;; Chipset registers
    ldb g1, g0          ;; Load memctl -> G1
    or g1, 1            ;; Set CG
    stb g0, g1          ;; Write it back

    mov g0, 0x116000    ;; Main memory
    mov g1, 0xBEEF      ;; Random word
    stw g0, g1          ;; Write it
    hlt

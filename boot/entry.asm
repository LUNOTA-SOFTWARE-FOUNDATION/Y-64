;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

boh_reserved: .byte 0x00, 0x00
boh_size: .byte @, 0x00
boh_reserved: .byte 0x00, 0x00, 0x00, 0x00

_start:
    mov sp, 0x116500
    hlt

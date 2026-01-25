;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

_start:
    mov g0, 0x100000        ;; Local cache
    mov g1, 0xFFFF          ;; Fill a single word
    stw g0, g1              ;; Load it

    mov g0, 0x110001        ;; Chipset registers [SPICTL]
    mov g1, prpd            ;; Physical region page descriptor
    stq g0, g1              ;; Post PRPD to SPI controller
    hlt

prpd:
prpd_buf:       .byte 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
prpd_len:       .byte 0x10, 0x00
prpd_chipsel:   .byte 0x00
prpd_write:     .byte 0x01
prpd_offset:    .byte 0x00, 0x00

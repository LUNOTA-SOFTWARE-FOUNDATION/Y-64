;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

_start:
    ;; Disable interrupts
    mov g1, 0x01        ;; IntConf
    srr                 ;; -> G0
    or g0, 1            ;; Set interrupt mask
    srw                 ;; Write it back

    ;; Bring up external RAM
    mov g0, 0x110000    ;; Chipset register base
    ldb g1, g0          ;; MEMCTL -> G1
    or g1, 1            ;; MEMCTL.CG (cache gate enable)
    stb g0, g1          ;; Write it back

    ;; Load bootloader
    mov g0, 0x110001    ;; Chipset SPICTL base
    mov g1, rd_prpd     ;; Read-op PRPD
    stq g0, g1          ;; Post read

    ;; TODO: WAIT FOR CONTROLLER TO READY
    ;; TODO: Hardware bring-up, RAM check and magic check

    mov g0, 0x116000    ;; Bootloader entrypoint
    b g0

;;
;; MicroSD physical region descriptor page, the bootloader
;; shall be at offset zero and loaded to address 0x116000
;;
;; TODO: We are currently only loading 512 bytes, this must
;;       be variable length.
;;
rd_prpd:         .byte 0x00, 0x60, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00
rd_prpd_len:     .byte 0x00, 0x02
rd_prpd_chipsel: .byte 0x00
rd_prpd_write:   .byte 0x00
rd_prpd_off:     .byte 0x00, 0x00

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

    ;; Construct a PRDP
    mov g0, 0x100000    ;; PD lcache [prpd buffer]
    mov g1, 0x116000    ;; Bootloader header
    stq g0, g1          ;; Write it

    mov g0, 0x100008    ;; PD lcache [prpd length]
    mov g1, 0x8         ;; Eight bytes for header
    stw g0, g1          ;; Write it

    mov g0, 0x10000A    ;; PD lcache [prpd write]
    mov g1, 0x0         ;; Read operation
    stw g0, g1          ;; Write it

    mov g0, 0x10000C    ;; PD lcache [prpd off]
    stw g0, g1          ;; Write it

    ;; Load bootloader
    mov g0, 0x110001    ;; Chipset SPICTL base
    mov g1, 0x100000    ;; Read-op PRPD
    stq g0, g1          ;; Post read

    ;; TODO: WAIT FOR CONTROLLER TO READY
    ;; TODO: Hardware bring-up, RAM check and magic check

    ;; Update total length
    mov g0, 0x116002
    ldw g1, g0
    mov g0, 0x100008
    stw g0, g1

    ;; Load rest of bootloader
    mov g0, 0x110001    ;; Chipset SPICTL base
    mov g1, 0x100000    ;; Read-op PRPD
    stq g0, g1          ;; Post read

    mov g0, 0x116008    ;; Bootloader entrypoint
    b g0

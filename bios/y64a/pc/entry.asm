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

    ;;
    ;; TODO: Verify external RAM is present and functional,
    ;;       bring up hardware and load bootloader from the
    ;;       SD card.
    ;;
    hlt

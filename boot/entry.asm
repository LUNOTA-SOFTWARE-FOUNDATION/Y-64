;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

;;
;; BIOS loads the loader to the start of usable RAM which exists
;; at address 0x116000.
;;
.org 0x116000

;;
;; The beginning of the boot block is to contain a BIOS hand-off
;; header (BOH) which provides the BIOS with information about
;; the loader and its requirements.
;;
boh_magic:      .word 0x2E55        ;; Magic number
boh_ldsz:       .quad end - _start  ;; Total size of loader in bytes.
boh_reserved:   .short 0            ;; Reserved for future use.
boh_reserved1:  .long  0            ;; Reserved for future use.

_start:
    ;;
    ;; Disable interrupts in case the firmware in-use didn't
    ;; do so... This should not happen on stock firmware, however
    ;; this assumption cannot be maintained for a non-stock BIOS.
    ;;
    mov g1, 0x01        ;; IntConf
    srr                 ;; -> G0
    or g0, 1            ;; IntMask
    srw

    ;;
    ;; TODO: Load a kernel or boot payload.
    ;;
    hlt

;;
;; Marks the end of the loader so the size can be computed
;; in the BOH.
;;
end:

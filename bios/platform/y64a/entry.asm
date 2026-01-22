;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

_start:
    ;;
    ;; Ensure that interrupts are disabled, this can be
    ;; done with the interrupt mask bit of special register
    ;; one.
    ;;
    mov g1, 0x00000001
    srr
    or g0, 1
    srw

    ;;
    ;; Next, we need a stack... This early in, we cannot
    ;; access external RAM so we'll need to put it at the
    ;; top of the processor local cache. And remember that
    ;; the stack grows down.
    ;;
    mov sp, 0x110000

    ;;
    ;; TODO: The interrupt service table needs to be placed at the bottom
    ;;       of the processor local cache
    ;;
halt:
    hlt
    b halt

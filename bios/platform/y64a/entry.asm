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

    ;; - -TODO: Some hardware bring up needs to be done --

    ;;
    ;; Bring up external RAM by asserting the cache gate
    ;; line through the chipset registers (@ 0x110000).
    ;;
    mov g0, 0x110000        ;; Chipset register base [CRB]
    ldb g1, g0              ;; *CRB -> G1
    or g1, 1                ;; Cache gate (CG)
    stb g0, g1              ;; Write it back

    ;;
    ;; If the bit we wrote didn't stick, the board does not have
    ;; external RAM. If this occurs and we are an embedded chip
    ;; revision, we can simply rely on the 64K lcache that we are
    ;; using now... AFTER WE FLUSH IT OF COURSE! Side-channels exist...
    ;;
    ;; However, if we are on a chip revision intended for personal
    ;; computing, we should probably yield a POST failure.
    ;;
    ldb g1, g0      ;; *CRB -> G1
    test g1, 1      ;; Cache gate (CG), do we have RAM?
    bs have_ram     ;; Yes, no need to take potential action
    hlt             ;; Halt for now (TODO: POST FAIL / INDICATE)
have_ram:

    ;;
    ;; TODO: The interrupt service table needs to be placed at the bottom
    ;;       of the processor local cache
    ;;
    hlt

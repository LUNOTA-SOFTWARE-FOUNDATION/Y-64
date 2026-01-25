;;
;; Copyright (c) 2026, Ian Moffett.
;; Provided under the BSD-3 clause.
;;

_start:
    mov g1, 0x01        ;; IntConf
    srr                 ;; -> G0
    or g0, 1            ;; Set interrupt mask
    srw                 ;; Write it back

    ;;
    ;; TODO: Bring up hardware for embedded chip
    ;;       revisions
    ;;
    hlt

/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_TOKEN_H
#define ARKI_TOKEN_H 1

#include <sys/types.h>
#include <stdint.h>

/*
 * Represents valid token types
 */
typedef enum {
    TT_NONE,        /* <NONE> */
    TT_IDENT,       /* <IDENT> */
    TT_NUMBER,      /* <NUMBER> */
    TT_COMMENT,     /* <COMMENT> */
    TT_LABEL,       /* <LABEL> */
    TT_COMMA,       /* ',' */
    TT_NEWLINE,     /* '\n' */
    TT_MOV,         /* 'mov' */
    TT_G0,          /* 'g0' */
    TT_G1,          /* 'g1' */
    TT_G2,          /* 'g2' */
    TT_G3,          /* 'g3' */
    TT_G4,          /* 'g4' */
    TT_G5,          /* 'g5' */
    TT_G6,          /* 'g6' */
    TT_G7,          /* 'g7' */
    TT_A0,          /* 'a0' */
    TT_A1,          /* 'a1' */
    TT_A2,          /* 'a2' */
    TT_A3,          /* 'a3' */
    TT_A4,          /* 'a4' */
    TT_A5,          /* 'a5' */
    TT_A6,          /* 'a6' */
    TT_A7,          /* 'a7' */
    TT_SP,          /* 'sp' */
    TT_HLT,         /* 'hlt' */
    TT_SRR,         /* 'srr' */
    TT_SRW,         /* 'srw' */
    TT_OR,          /* 'or' */
    TT_LITR,        /* 'litr' */
    TT_STB,         /* 'stb' */
    TT_STW,         /* 'stw' */
    TT_STL,         /* 'stl' */
    TT_STQ,         /* 'stq' */
    TT_LDB,         /* 'ldb' */
    TT_LDW,         /* 'ldw' */
    TT_LDL,         /* 'ldl' */
    TT_LDQ,         /* 'ldq' */
    TT_B,           /* 'b' */
    TT_BYTE,        /* '.byte' */
    TT_SKIP,        /* '.skip' */
} tt_t;

/*
 * Represents a lexical token
 */
struct token {
    tt_t type;
    union {
        char c;
        ssize_t v;
        char *s;
    };
};

#endif  /* !ARKI_TOKEN_H */

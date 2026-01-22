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
    TT_COMMA,       /* ',' */
    TT_NEWLINE,     /* '\n' */
    TT_MOV,         /* 'mov' */
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

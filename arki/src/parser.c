/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include "arki/parser.h"
#include "arki/token.h"
#include "arki/lexer.h"

/* Convert token type to string */
#define tokstr1(tt) \
    toktab[(tt)]

/* Convert token to string */
#define tokstr(tok) \
    tokstr1((tok)->type)

/* Quoted token */
#define qtok(tokstr) \
    "'" tokstr "'"

/* Symbolic token */
#define symtok(tokstr) \
    "<" tokstr ">"

/*
 * A lookup table used to convert token types into
 * human readable strings
 */
static const char *toktab[] = {
    [TT_NONE]       = symtok("none"),
    [TT_IDENT]      = symtok("ident"),
    [TT_COMMA]      = qtok(","),
    [TT_NEWLINE]    = symtok("newline")
};

int
arki_parse(struct arki_state *state)
{
    if (state == NULL) {
        return -1;
    }

    while (lexer_scan(state, &state->last_tok) == 0) {
        printf("got token %s\n", tokstr(&state->last_tok));
    }

    return 0;
}

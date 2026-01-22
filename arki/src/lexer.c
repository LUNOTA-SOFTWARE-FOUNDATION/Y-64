/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include "arki/lexer.h"

/*
 * Returns true if the given character counts
 * as a whitespace.
 */
static inline bool
lexer_is_ws(char c)
{
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
    case '\f':
        return true;
    }

    return false;
}

/*
 * Consume a single byte from the input file
 *
 * @state:   Assembler state
 * @skip_ws: If true, skip whitespace characters
 *
 * Returns the consumed character on success, otherwise
 * '\0' on failure or EOF.
 */
static char
lexer_consume(struct arki_state *state, bool skip_ws)
{
    char c;

    /* Begin reading bytes and skip whitespace if we can */
    while (read(state->in_fd, &c, 1) > 0) {
        if (skip_ws && lexer_is_ws(c)) {
            continue;
        }

        return c;
    }

    return '\0';
}

int
lexer_scan(struct arki_state *state, struct token *res)
{
    char c;

    if (res == NULL) {
        return -1;
    }

    if ((c = lexer_consume(state, true)) == '\0') {
        return -1;
    }

    switch (c) {
    case ',':
        res->type = TT_COMMA;
        res->c = c;
        return 0;
    case '\n':
        res->type = TT_NEWLINE;
        res->c = c;
        return 0;
    default:
        printf("fatal: unexpected token '%c'\n", c);
        break;
    }

    return -1;
}

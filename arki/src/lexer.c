/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include "arki/lexer.h"
#include "arki/ptrbox.h"

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

/*
 * Scan for a single identifier
 *
 * @state:  Assembler state
 * @lc:     Last character
 * @re:     Token result
 *
 * Returns zero on success
 */
static int
lexer_scan_ident(struct arki_state *state, int lc, struct token *res)
{
    char c, *buf;
    size_t bufcap, bufind;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (!isalpha(lc) && lc != '_') {
        errno = -EINVAL;
        return -1;
    }

    /* Set buffer capacity and index */
    bufcap = 8;
    bufind = 0;

    if ((buf = malloc(bufcap)) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    buf[bufind++] = lc;
    for (;;) {
        c = lexer_consume(state, false);
        if (!isalpha(c)) {
            buf[bufind] = '\0';
            break;
        }

        /* Fill and resize buffer if needed */
        buf[bufind++] = c;
        if (bufind >= bufcap - 1) {
            bufcap += 8;
            buf = realloc(buf, bufcap);
        }

        if (buf == NULL) {
            errno = -ENOMEM;
            return -1;
        }
    }

    res->type = TT_IDENT;
    res->s = ptrbox_strdup(&state->ptrbox, buf);
    free(buf);
    return 0;
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
        if (lexer_scan_ident(state, c, res) == 0) {
            return 0;
        }

        printf("fatal: unexpected token '%c'\n", c);
        break;
    }

    return -1;
}

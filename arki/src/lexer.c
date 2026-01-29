/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include "arki/lexer.h"
#include "arki/ptrbox.h"
#include "arki/trace.h"

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
 * Place a given character into the lexer putback buffer
 *
 * @state: Assembler state
 * @c:     Character to insert
 */
static inline void
lexer_putback(struct arki_state *state, char c)
{
    if (state == NULL) {
        return;
    }

    state->putback = c;
}

/*
 * Skip an entire line until a newline
 *
 * @state: Assembler state
 */
static void
lexer_skip_line(struct arki_state *state)
{
    char c;

    if (state == NULL) {
        return;
    }

    while (read(state->in_fd, &c, 1) > 0) {
        if (c == '\n') {
            break;
        }
    }
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

    /* Is this anything in the putback buffer? */
    if (state->putback != '\0') {
        c = state->putback;
        state->putback = '\0';

        if (lexer_is_ws(c) && !skip_ws)
            return c;
        if (!lexer_is_ws(c))
            return c;
    }

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

    switch (lc) {
    case '_':
    case '.':
        break;
    default:
        if (!isalpha(lc)) {
            errno = -EINVAL;
            return -1;
        }

        break;
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
        if (!isalnum(c) && c != '_') {
            buf[bufind] = '\0';
            if (c != ':') lexer_putback(state, c);
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

    res->type = c != ':' ? TT_IDENT : TT_LABEL;
    res->s = ptrbox_strdup(&state->ptrbox, buf);
    free(buf);
    return 0;
}

/*
 * Check an indentifier token against a list of  registers and
 * override the type if it matches.
 *
 * @tok:   Token to check
 *
 * Returns zero on success
 */
static int
lexer_reg(struct token *tok)
{
    uint8_t reg_num;
    char *p;
    tt_t *table_ptr = NULL;
    tt_t areg_lookup[] = {
        TT_A0,   TT_A1,
        TT_A2,   TT_A3,
        TT_A4,   TT_A5,
        TT_A6,   TT_A7
    };
    tt_t greg_lookup[] = {
        TT_G0,   TT_G1,
        TT_G2,   TT_G3,
        TT_G4,   TT_G5,
        TT_G6,   TT_G7
    };

    if (tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_IDENT) {
        errno = -EINVAL;
        return -1;
    }

    p = tok->s;
    switch (*(p++))  {
    case 'g':
        table_ptr = &greg_lookup[0];
        break;
    case 'a':
        table_ptr = &areg_lookup[0];
        break;
    }

    /* There are only 7 a/g regs each */
    reg_num = atoi(p);
    if (reg_num > 7) {
        errno = -EINVAL;
        return -1;
    }

    tok->type = table_ptr[reg_num];
    return 0;
}

/*
 * Check if a token is actually a keyword and overwrite it if
 * so
 *
 * @tok: Token to check
 *
 * Returns zero on successful overwrite
 */
static int
lexer_check_kw(struct token *tok)
{
    if (tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_IDENT) {
        errno = -EINVAL;
        return -1;
    }

    switch (*tok->s) {
    case 'm':
        if (strcmp(tok->s, "mov") == 0) {
            tok->type = TT_MOV;
            return 0;
        }

        break;
    case 'h':
        if (strcmp(tok->s, "hlt") == 0) {
            tok->type = TT_HLT;
            return 0;
        }

        break;
    case 's':
        if (strcmp(tok->s, "sp") == 0) {
            tok->type = TT_SP;
            return 0;
        }

        if (strcmp(tok->s, "srr") == 0) {
            tok->type = TT_SRR;
            return 0;
        }

        if (strcmp(tok->s, "srw") == 0) {
            tok->type = TT_SRW;
            return 0;
        }

        if (strcmp(tok->s, "stb") == 0) {
            tok->type = TT_STB;
            return 0;
        }

        if (strcmp(tok->s, "stw") == 0) {
            tok->type = TT_STW;
            return 0;
        }

        if (strcmp(tok->s, "stl") == 0) {
            tok->type = TT_STL;
            return 0;
        }

        if (strcmp(tok->s, "stq") == 0) {
            tok->type = TT_STQ;
            return 0;
        }

        break;
    case 'o':
        if (strcmp(tok->s, "or") == 0) {
            tok->type = TT_OR;
            return 0;
        }

        break;
    case 'l':
        if (strcmp(tok->s, "litr") == 0) {
            tok->type = TT_LITR;
            return 0;
        }

        if (strcmp(tok->s, "ldb") == 0) {
            tok->type = TT_LDB;
            return 0;
        }

        if (strcmp(tok->s, "ldw") == 0) {
            tok->type = TT_LDW;
            return 0;
        }

        if (strcmp(tok->s, "ldl") == 0) {
            tok->type = TT_LDL;
            return 0;
        }

        if (strcmp(tok->s, "ldq") == 0) {
            tok->type = TT_LDQ;
            return 0;
        }

        break;
    case '.':
        if (strcmp(tok->s, ".byte") == 0) {
            tok->type = TT_BYTE;
            return 0;
        }

        if (strcmp(tok->s, ".skip") == 0) {
            tok->type = TT_SKIP;
            return 0;
        }

        break;
    case 'b':
        if (strcmp(tok->s, "b") == 0) {
            tok->type = TT_B;
            return 0;
        }

        break;
    case 'g':
    case 'a':
        if (lexer_reg(tok) == 0) {
            return 0;
        }

        break;
    }

    return -1;
}

/*
 * Scan for a number token
 *
 * @state:  Assembler state
 * @lc:     Last token
 * @tok:    Token result
 */
static int
lexer_scan_number(struct arki_state *state, int lc, struct token *res)
{
    char buf[24];
    size_t bufind = 0;
    uint8_t base = 10;
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (!isdigit(lc)) {
        errno = -EINVAL;
        return -1;
    }

    /*
     * If we have a '0' prefix, then we are using a non base-10
     * numerical:
     *
     * 'x')
     *      Base-16
     */
    if (lc == '0') {
        c = lexer_consume(state, false);
        if (c == 'x') {
            base = 16;
            c = lexer_consume(state, false);
        }
    }

    if (lc != '0') {
        buf[bufind++] = lc;
    } else {
        buf[bufind++] = c;
    }

    for (;;) {
        c = lexer_consume(state, false);
        if (base == 16 && !isxdigit(c)) {
            if (c == '_') continue;
            buf[bufind] = '\0';
            lexer_putback(state, c);
            break;
        }

        if (base == 10 && !isdigit(c)) {
            if (c == '_') continue;
            buf[bufind] = '\0';
            lexer_putback(state, c);
            break;
        }

        buf[bufind++] = c;
        if (bufind >= sizeof(buf) - 1) {
            buf[bufind] = '\0';
            break;
        }
    }

    res->type = TT_NUMBER;
    res->v = strtol(buf, NULL, base);
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
        ++state->line_num;
        res->type = TT_NEWLINE;
        res->c = c;
        return 0;
    case ';':
        lexer_skip_line(state);
        res->type = TT_COMMENT;
        res->c = c;
        return 0;
    default:
        if (lexer_scan_ident(state, c, res) == 0) {
            lexer_check_kw(res);
            return 0;
        }

        if (lexer_scan_number(state, c, res) == 0) {
            return 0;
        }

        trace_error(state, "unexpected token '%c'\n", c);
        break;
    }

    return -1;
}

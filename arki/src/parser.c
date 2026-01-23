/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include "arki/parser.h"
#include "arki/token.h"
#include "arki/lexer.h"
#include "arki/trace.h"
#include "arki/codegen.h"
#include "arki/reg.h"

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

/* Unexpected token */
#define utok(state, tok)            \
    trace_error(                    \
        (state),                    \
        "unexpected token %s\n",    \
        tokstr(tok)                 \
    );

/* Unexpected token */
#define utok1(state, exp, got)           \
    trace_error(                         \
        (state),                         \
        "expected %s, got %s instead\n", \
        (exp),                           \
        (got)                            \
    );

/* Unexpected EOF */
#define ueof(state)                     \
    trace_error(                        \
        (state),                        \
        "unexpected end of of file\n"   \
    )

/*
 * A lookup table used to convert token types into
 * human readable strings
 */
static const char *toktab[] = {
    [TT_NONE]       = symtok("none"),
    [TT_IDENT]      = symtok("ident"),
    [TT_NUMBER]     = symtok("number"),
    [TT_COMMENT]    = symtok("comment"),
    [TT_COMMA]      = qtok(","),
    [TT_NEWLINE]    = symtok("newline"),
    [TT_MOV]        = qtok("mov"),
    [TT_G0]         = qtok("g0"),
    [TT_G1]         = qtok("g1"),
    [TT_G2]         = qtok("g2"),
    [TT_G3]         = qtok("g3"),
    [TT_G4]         = qtok("g4"),
    [TT_G5]         = qtok("g5"),
    [TT_G6]         = qtok("g6"),
    [TT_G7]         = qtok("g7"),
    [TT_A0]         = qtok("a0"),
    [TT_A1]         = qtok("a1"),
    [TT_A2]         = qtok("a2"),
    [TT_A3]         = qtok("a3"),
    [TT_A4]         = qtok("a4"),
    [TT_A5]         = qtok("a5"),
    [TT_A6]         = qtok("a6"),
    [TT_A7]         = qtok("a7"),
    [TT_HLT]        = qtok("hlt"),
    [TT_SRR]        = qtok("srr"),
    [TT_SRW]        = qtok("srw")
};

/*
 * Parser-side token scan function
 *
 * @state:  Assembler state
 * @tok:    Last token
 *
 * Returns zero on success
 */
static int
parse_scan(struct arki_state *state, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        return -1;
    }

    return 0;
}

/*
 * Assert that the next token is of a specific type
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @what:   Token to expect
 */
static int
parse_expect(struct arki_state *state, struct token *tok, tt_t what)
{
    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if (tok->type != what) {
        utok1(state, tokstr1(what), tokstr(tok));
        return -1;
    }

    return 0;
}

/*
 * Parse a 'mov' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:   AST node result
 *
 * Returns zero on success
 */
static int
parse_mov(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root;
    struct ast_node *left;
    struct ast_node *right;
    reg_t rd, rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_MOV) {
        return -1;
    }

    if (ast_alloc_node(state, AST_MOV, &root) < 0) {
        trace_error(state, "failed to allocate AST_MOV\n");
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    /* EXPECT <register> */
    if ((rd = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
        return -1;
    }

    if (ast_alloc_node(state, AST_REG, &left) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    /* EXPECT ',' */
    if (parse_expect(state, tok, TT_COMMA) < 0) {
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    left->reg = rd;
    switch (tok->type) {
    case TT_NUMBER:
        if (ast_alloc_node(state, AST_NUMBER, &right) < 0) {
            trace_error(state, "failed to allocate AST_NUMBER\nn");
            return -1;
        }

        right->v = tok->v;
        break;
    default:
        /* EXPECT <register> */
        if ((rs = token_to_reg(tok->type)) == REG_BAD) {
            utok1(state, symtok("register"), tokstr(tok));
            return -1;
        }

        if (ast_alloc_node(state, AST_REG, &right) < 0) {
            trace_error(state, "failed to allocate AST_REG\n");
            return -1;
        }

        right->reg = rs;
        break;
    }

    root->left = left;
    root->right = right;
    *res = root;
    return 0;
}

/*
 * Parse a 'hlt' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_hlt(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_HLT) {
        return -1;
    }

    if (ast_alloc_node(state, AST_HLT, &root) < 0) {
        trace_error(state, "failed to allocate AST_HLT\n");
        return -1;
    }

    *res = root;
    return 0;
}

/*
 * Parse the 'srr' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_srr(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_SRR) {
        return -1;
    }

    if (ast_alloc_node(state, AST_SRR, &root) < 0) {
        trace_error(state, "failed to allocate AST_SRR\n");
        return -1;
    }

    *res = root;
    return 0;
}

/*
 * Parse the 'srw' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_srw(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_SRW) {
        return -1;
    }

    if (ast_alloc_node(state, AST_SRW, &root) < 0) {
        trace_error(state, "failed to allocate AST_SRW\n");
        return -1;
    }

    *res = root;
    return 0;
}

/*
 * Parse the last token
 *
 * @state:  Assembler state
 * @tok:    Last token
 *
 * Returns zero on susccess
 */
static int
parse_begin(struct arki_state *state, struct token *tok)
{
    struct ast_node *root = NULL;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    switch (tok->type) {
    case TT_MOV:
        if (parse_mov(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_HLT:
        if (parse_hlt(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_SRR:
        if (parse_srr(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_SRW:
        if (parse_srw(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_NEWLINE:
        /* Ignored */
        break;
    case TT_COMMENT:
        /* Ignored */
        break;
    default:
        utok(state, tok);
        return -1;
    }

    if (root != NULL) {
        if (cg_resolve_node(state, root) < 0)
            return -1;
    }

    return 0;
}

int
arki_parse(struct arki_state *state)
{
    if (state == NULL) {
        return -1;
    }

    while (lexer_scan(state, &state->last_tok) == 0) {
        if (parse_begin(state, &state->last_tok) < 0) {
            return -1;
        }
    }

    return 0;
}

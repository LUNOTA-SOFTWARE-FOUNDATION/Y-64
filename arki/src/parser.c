/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
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
    [TT_LABEL]      = symtok("label"),
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
    [TT_SP]         = qtok("sp"),
    [TT_HLT]        = qtok("hlt"),
    [TT_SRR]        = qtok("srr"),
    [TT_SRW]        = qtok("srw"),
    [TT_OR]         = qtok("or"),
    [TT_LITR]       = qtok("litr"),
    [TT_STB]        = qtok("stb"),
    [TT_STW]        = qtok("stw"),
    [TT_STL]        = qtok("stl"),
    [TT_STQ]        = qtok("stq"),
    [TT_LDB]        = qtok("ldb"),
    [TT_LDW]        = qtok("ldw"),
    [TT_LDL]        = qtok("ldl"),
    [TT_LDQ]        = qtok("ldq"),
    [TT_B]          = qtok("b"),
    [TT_BYTE]       = qtok(".byte")
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
 * Acquire a symbol by name
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    Symbol result
 *
 * XXX: Symbol might be NULL on first pass!
 *
 * Returns zero on success
 */
static int
parse_get_sym(struct arki_state *state, const char *name, struct symbol **res)
{
    struct symbol *sym;

    if (state == NULL || name == NULL) {
        return -1;
    }

    if (name == NULL) {
        return -1;
    }

    sym = symbol_by_name(&state->symtab, name);
    if (sym == NULL && state->pass_count > 0) {
        trace_error(state, "undefined reference to '%s'\n", name);
        return -1;
    }

    *res = sym;
    return 0;
}

/*
 * Parse source operand
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_source(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *rhs;
    struct symbol *sym;
    reg_t rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    switch (tok->type) {
    case TT_NUMBER:
        if (ast_alloc_node(state, AST_NUMBER, &rhs) < 0) {
            trace_error(state, "failed to allocate AST_NUMBER\nn");
            return -1;
        }

        rhs->v = tok->v;
        break;
    case TT_IDENT:
        if (parse_get_sym(state, tok->s, &sym) < 0) {
            return -1;
        }

        /* Handle first pass case */
        if (sym != NULL) {
            if (sym->type != SYMBOL_LABEL) {
                trace_error(state, "'%s' is not a label\n", tok->s);
                return -1;
            }
        }

        if (ast_alloc_node(state, AST_LABEL, &rhs) < 0) {
            trace_error(state, "failed to allocate AST_LABEL\n");
            return -1;
        }

        rhs->symbol = sym;
        break;
    default:
        /* EXPECT <register> */
        if ((rs = token_to_reg(tok->type)) == REG_BAD) {
            utok1(state, symtok("register"), tokstr(tok));
            return -1;
        }

        if (ast_alloc_node(state, AST_REG, &rhs) < 0) {
            trace_error(state, "failed to allocate AST_REG\n");
            return -1;
        }

        rhs->reg = rs;
        break;
    }

    *res = rhs;
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
    reg_t rd;

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
    if (parse_source(state, tok, &right) < 0) {
        return -1;
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
 * Parse the 'or' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_or(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *left, *root, *right;
    reg_t rd;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_OR) {
        return -1;
    }

    if (ast_alloc_node(state, AST_OR, &root) < 0) {
        trace_error(state, "failed to allocate AST_OR\n");
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
    if (parse_source(state, tok, &right) < 0) {
        return -1;
    }

    root->left = left;
    root->right = right;
    *res = root;
    return 0;
}

/*
 * Parse the 'litr' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_litr(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root;
    reg_t rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_LITR) {
        return -1;
    }

    if (ast_alloc_node(state, AST_LITR, &root) < 0) {
        trace_error(state, "could not allocate AST_LITR\n");
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if ((rs = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
        return -1;
    }

    root->reg = rs;
    *res = root;
    return 0;
}

/*
 * Parse a label
 *
 * @state: Assembler state
 * @tok:   Last token
 */
static int
parse_label(struct arki_state *state, struct token *tok)
{
    struct symbol *sym;
    int error;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (tok->type != TT_LABEL) {
        return -1;
    }

    if (state->pass_count == 0) {
        error = symbol_table_new(
            &state->symtab,
            tok->s,
            SYMBOL_LABEL,
            &sym
        );

        if (error < 0) {
            trace_error(state, "failed to allocate symbol\n");
            return -1;
        }

        sym->vpc = arki_get_vpc(state);
    }

    return 0;
}

/*
 * Parse the store variant instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_store(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *lhs, *root, *rhs;
    reg_t rd, rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    switch (tok->type) {
    case TT_STB:
        if (ast_alloc_node(state, AST_STB, &root) < 0) {
            trace_error(state, "failed to allocate AST_STB\n");
            return -1;
        }

        break;
    case TT_STW:
        if (ast_alloc_node(state, AST_STW, &root) < 0) {
            trace_error(state, "failed to allocate AST_STW\n");
            return -1;
        }

        break;
    case TT_STL:
        if (ast_alloc_node(state, AST_STL, &root) < 0) {
            trace_error(state, "failed to allocate AST_STL\n");
            return -1;
        }

        break;
    case TT_STQ:
        if (ast_alloc_node(state, AST_STQ, &root) < 0) {
            trace_error(state, "failed to allocate AST_STQ\n");
            return -1;
        }

        break;
    default:
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if ((rd = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
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

    if ((rs = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
        return -1;
    }

    if (ast_alloc_node(state, AST_REG, &lhs) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    if (ast_alloc_node(state, AST_REG, &rhs) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    lhs->reg = rd;
    rhs->reg = rs;

    root->left = lhs;
    root->right = rhs;
    *res = root;
    return 0;
}

/*
 * Parse the load variant instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_load(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *lhs, *root, *rhs;
    reg_t rd, rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    switch (tok->type) {
    case TT_LDB:
        if (ast_alloc_node(state, AST_LDB, &root) < 0) {
            trace_error(state, "failed to allocate AST_LDB\n");
            return -1;
        }

        break;
    case TT_LDW:
        if (ast_alloc_node(state, AST_LDW, &root) < 0) {
            trace_error(state, "failed to allocate AST_LDW\n");
            return -1;
        }

        break;
    case TT_LDL:
        if (ast_alloc_node(state, AST_LDL, &root) < 0) {
            trace_error(state, "failed to allocate AST_LDL\n");
            return -1;
        }

        break;
    case TT_LDQ:
        if (ast_alloc_node(state, AST_LDQ, &root) < 0) {
            trace_error(state, "failed to allocate AST_LDQ\n");
            return -1;
        }

        break;
    default:
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if ((rs = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
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

    if ((rd = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
        return -1;
    }

    if (ast_alloc_node(state, AST_REG, &lhs) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    if (ast_alloc_node(state, AST_REG, &rhs) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    lhs->reg = rs;
    rhs->reg = rd;

    root->left = lhs;
    root->right = rhs;
    *res = root;
    return 0;
}

/*
 * Parse a '.byte' directive
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_byte(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root, *cur;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_BYTE) {
        return -1;
    }

    if (ast_alloc_node(state, AST_BYTE, &root) < 0) {
        trace_error(state, "failed to allocate AST_BYTE\n");
        return -1;
    }

    if (parse_expect(state, tok, TT_NUMBER) < 0) {
        return -1;
    }

    cur = root;
    while (tok->type == TT_NUMBER) {
        if (ast_alloc_node(state, AST_NUMBER, &cur->right) < 0) {
            trace_error(state, "failed to allocate AST_NUMBER\n");
            return -1;
        }

        cur = cur->right;
        cur->v = tok->v;

        if (parse_scan(state, tok) < 0) {
            ueof(state);
            return -1;
        }

        if (tok->type == TT_NEWLINE) {
            break;
        }

        if (tok->type != TT_COMMA) {
            utok1(state, tokstr1(TT_COMMA), tokstr(tok));
            return -1;
        }

        if (parse_expect(state, tok, TT_NUMBER) < 0) {
            return -1;
        }
    }

    *res = root;
    return 0;
}

/*
 * Parse a 'b' instruction
 *
 * @state:  Assembler state
 * @tok:    Last token
 * @res:    AST node result
 *
 * Returns zero on success
 */
static int
parse_branch(struct arki_state *state, struct token *tok, struct ast_node **res)
{
    struct ast_node *root, *rhs;
    reg_t rs;

    if (state == NULL || tok == NULL) {
        return -1;
    }

    if (res == NULL) {
        return -1;
    }

    if (tok->type != TT_B) {
        return -1;
    }

    if (parse_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    /* EXPECT <register> */
    if ((rs = token_to_reg(tok->type)) == REG_BAD) {
        utok1(state, symtok("register"), tokstr(tok));
        return -1;
    }

    if (ast_alloc_node(state, AST_BRANCH, &root) < 0) {
        trace_error(state, "failed to allocate AST_BRANCH\n");
        return -1;
    }

    root->type = AST_BRANCH;
    if (ast_alloc_node(state, AST_REG, &rhs) < 0) {
        trace_error(state, "failed to allocate AST_REG\n");
        return -1;
    }

    rhs->reg = rs;
    root->right = rhs;
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
    case TT_OR:
        if (parse_or(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_LITR:
        if (parse_litr(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_STB:
    case TT_STW:
    case TT_STL:
    case TT_STQ:
        if (parse_store(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_LDB:
    case TT_LDW:
    case TT_LDL:
    case TT_LDQ:
        if (parse_load(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_BYTE:
        if (parse_byte(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_B:
        if (parse_branch(state, tok, &root) < 0) {
            return -1;
        }

        break;
    case TT_NEWLINE:
        /* Ignored */
        break;
    case TT_COMMENT:
        /* Ignored */
        break;
    case TT_LABEL:
        if (parse_label(state, tok) < 0) {
            return -1;
        }

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

    ++state->pass_count;
    state->line_num = 1;
    state->vpc = 0;
    lseek(state->in_fd, 0, SEEK_SET);
    return 0;
}

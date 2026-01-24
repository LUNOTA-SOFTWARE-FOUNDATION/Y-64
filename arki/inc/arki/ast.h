/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_AST_H
#define ARKI_AST_H 1

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include "arki/state.h"
#include "arki/reg.h"

/*
 * Represents valid AST node types
 *
 * @AST_NONE:   This node has no type
 * @AST_MOV:    This node is a 'MOV' instruction
 * @AST_HLT:    This node is a 'HLT' instruction
 * @AST_SRR:    This node is a 'SRR' instruction
 * @AST_SRW:    This node is a 'SRW' instruction
 * @AST_OR:     This node is a 'OR' instruction
 * @AST_LITR:   This node is a 'LITR' instruction
 * @AST_STB:    This node is a 'STB' instruction
 * @AST_STW:    This node is a 'STW' instruction
 * @AST_STL:    This node is a 'STL' instruction
 * @AST_STQ:    This node is a 'STQ' instruction
 * @AST_LDB:    This node is a 'LDB' instruction
 * @AST_LDW:    This node is a 'LDW' instruction
 * @AST_LDL:    This node is a 'LDL' instruction
 * @AST_LDQ:    This node is a 'LDQ' instruction
 * @AST_NUMBER: This node is a number
 * @AST_REG:    This node is a register
 */
typedef enum {
    AST_NONE,
    AST_MOV,
    AST_HLT,
    AST_SRR,
    AST_SRW,
    AST_OR,
    AST_LITR,
    AST_STB,
    AST_STW,
    AST_STL,
    AST_STQ,
    AST_LDB,
    AST_LDW,
    AST_LDL,
    AST_LDQ,
    AST_NUMBER,
    AST_REG,
} ast_type_t;

/*
 * Represents an abstract syntax tree (AST) node
 *
 * @type:   Node type
 * @left:   Left-hand leaf
 * @right:  Right-hand leaf
 */
struct ast_node {
    ast_type_t type;
    struct ast_node *left;
    struct ast_node *right;
    union {
        ssize_t v;
        reg_t reg;
    };
};

/*
 * Allocate an AST node
 *
 * @state:  Assembler state
 * @type:   Node type
 * @res:    Result is written here
 *
 * Returns zero on success
 */
int ast_alloc_node(struct arki_state *state, ast_type_t type, struct ast_node **res);

#endif  /* !ARKI_AST_H */

/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_AST_H
#define ARKI_AST_H 1

#include <stdint.h>
#include <stddef.h>
#include "arki/state.h"

/*
 * Represents valid AST node types
 *
 * @AST_NONE:   This node has no type
 * @AST_MOV:    This node is a 'MOV' instruction
 * @AST_NUMBER: This node is a number
 */
typedef enum {
    AST_NONE,
    AST_MOV,
    AS_NUMBER
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

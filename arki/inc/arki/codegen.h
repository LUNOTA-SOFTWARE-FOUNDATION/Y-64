/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_CODEGEN_H
#define ARKI_CODEGEN_H 1

#include <stdint.h>
#include "arki/ast.h"
#include "arki/state.h"

/*
 * Resolve an abstract syntax tree root and generate
 * machine code
 *
 * @state: Assembler state machine
 * @root:  AST root node
 *
 * Returns zero on success
 */
int cg_resolve_node(struct arki_state *state, struct ast_node *root);

#endif  /* !ARKI_CODEGEN_H */

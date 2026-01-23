/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include "arki/codegen.h"
#include "arki/trace.h"

/* Various parameters */
#define SHORT_IMM_MAX (1 << 16)

/* Valid opcodes */
#define OPC_WMOV  0x01  /* Wide IMM move */
#define OPC_SMOV  0x03  /* Short IMM register load */

#define cg_emitb(state, byte) do {          \
        uint8_t b = (byte);                 \
                                            \
        write((state)->out_fd, &b, 1);      \
    } while (0);

/*
 * Generate code for the 'mov' instruction
 *
 * @state: Assembler state
 * @root:  Root node to resolve
 *
 * Returns zero on success
 */
static int
cg_emit_mov(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *lhs, *rhs;
    uint8_t opcode = OPC_SMOV;

    if (state == NULL || root == NULL) {
        return -1;
    }

    lhs = root->left;
    rhs = root->right;

    if (lhs->type != AST_REG) {
        trace_error(state, "lhs of mov is not a register\n");
        return -1;
    }

    /* TODO: Support register moves */
    if (rhs->type != AST_NUMBER) {
        trace_error(state, "rhs of mov is not an imm\n");
        return -1;
    }

    /* Should we use a wide move? */
    if (rhs->v > SHORT_IMM_MAX) {
        opcode = OPC_WMOV;
    }

    if (lhs->reg >= REG_MAX) {
        trace_error(state, "bad lhs register\n");
        return -1;
    }

    cg_emitb(state, opcode);
    cg_emitb(state, lhs->reg);
    while (rhs->v != 0) {
        cg_emitb(state, rhs->v & 0xFF);
        rhs->v >>= 8;
    }
    return 0;
}

int
cg_resolve_node(struct arki_state *state, struct ast_node *root)
{
    if (root == NULL) {
        return -1;
    }

    switch (root->type) {
    case AST_MOV:
        if (cg_emit_mov(state, root) < 0) {
            return -1;
        }

        return 0;
    default:
        trace_error(state, "bad AST node %d\n", root->type);
        return -1;
    }

    return 0;
}

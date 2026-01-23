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
#define OPC_HLT   0x0D  /* Halt processor */
#define OPC_SRR   0x0E  /* Special register read */
#define OPC_SRW   0x0F  /* Special register write */
#define OPC_IOR   0x10  /* IMM OR */

#define cg_emitb(state, byte) do {          \
        uint8_t b = (byte);                 \
                                            \
        if ((state)->pass_count == 1) {     \
            write((state)->out_fd, &b, 1);  \
        }                                   \
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
    size_t byte_count = 0;
    size_t max_bytes = 2;

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
        max_bytes = 6;
    }

    if (lhs->reg >= REG_MAX) {
        trace_error(state, "bad lhs register\n");
        return -1;
    }

    cg_emitb(state, opcode);
    cg_emitb(state, lhs->reg);
    while (rhs->v != 0 || byte_count < max_bytes) {
        cg_emitb(state, rhs->v & 0xFF);
        rhs->v >>= 8;
        ++byte_count;
    }
    return 0;
}

/*
 * Generate code for the 'hlt' instruction
 *
 * @state: Assembler state
 * @root:  Root node to resolve
 *
 * Returns zero on success
 */
static int
cg_emit_hlt(struct arki_state *state, struct ast_node *root)
{
    if (state == NULL || root == NULL) {
        return -1;
    }

    cg_emitb(state, OPC_HLT);
    return 0;
}

/*
 * Generate code for the 'srr' instruction
 *
 * @state: Assembler state
 * @root:  Root node to resolve
 *
 * Returns zero on success
 */
static int
cg_emit_srr(struct arki_state *state, struct ast_node *root)
{
    if (state == NULL || root == NULL) {
        return -1;
    }

    cg_emitb(state, OPC_SRR);
    return 0;
}

/*
 * Generate code for the 'srr' instruction
 *
 * @state: Assembler state
 * @root:  Root node to resolve
 *
 * Returns zero on success
 */
static int
cg_emit_srw(struct arki_state *state, struct ast_node *root)
{
    if (state == NULL || root == NULL) {
        return -1;
    }

    cg_emitb(state, OPC_SRW);
    return 0;
}

/*
 * Generate code for the 'or' instruction
 *
 * @state: Assembler state
 * @root:  Root node to resolve
 *
 * Returns zero on success
 */
static int
cg_emit_or(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *lhs, *rhs;
    size_t byte_count = 0;

    if (state == NULL || root == NULL) {
        return -1;
    }

    lhs = root->left;
    rhs = root->right;

    if (lhs->type != AST_REG) {
        trace_error(state, "lhs of mov is not a register\n");
        return -1;
    }

    /* TODO: Support register ORs */
    if (rhs->type != AST_NUMBER) {
        trace_error(state, "rhs of mov is not an imm\n");
        return -1;
    }

    if (lhs->reg >= REG_MAX) {
        trace_error(state, "bad lhs register\n");
        return -1;
    }

    cg_emitb(state, OPC_IOR);
    cg_emitb(state, lhs->reg);
    while (rhs->v != 0 || byte_count < 2) {
        cg_emitb(state, rhs->v & 0xFF);
        rhs->v >>= 8;
        ++byte_count;
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
    case AST_HLT:
        if (cg_emit_hlt(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_SRR:
        if (cg_emit_srr(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_SRW:
        if (cg_emit_srw(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_OR:
        if (cg_emit_or(state, root) < 0) {
            return -1;
        }

        return 0;
    default:
        trace_error(state, "bad AST node %d\n", root->type);
        return -1;
    }

    return 0;
}

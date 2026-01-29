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
#define OPC_LITR  0x14  /* Load ITR */
#define OPC_STB   0x15  /* Store byte */
#define OPC_STW   0x16  /* Store word */
#define OPC_STL   0x17  /* Store dword */
#define OPC_STQ   0x18  /* Store qword */
#define OPC_LDB   0x19  /* Load byte */
#define OPC_LDW   0x1A  /* Load word */
#define OPC_LDL   0x1B  /* Load dword */
#define OPC_LDQ   0x1C  /* Load qword */
#define OPC_B     0x1D  /* Indirect branch */

#define cg_emitb(state, byte) do {          \
        uint8_t b = (byte);                 \
                                            \
        if ((state)->pass_count == 1) {     \
            write((state)->out_fd, &b, 1);  \
        }                                   \
                                            \
        ++(state)->vpc;                     \
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
    struct symbol *symbol;
    struct ast_node *lhs, *rhs;
    uint8_t opcode = OPC_SMOV;
    size_t byte_count = 0;
    size_t max_bytes = 2;
    uintptr_t imm = 0;

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
    switch (rhs->type) {
    case AST_NUMBER:
        imm = rhs->v;
        break;
    case AST_LABEL:
        if ((symbol = rhs->symbol) == NULL && state->pass_count > 0) {
            trace_error(state, "mov rhs hsa no symbol\n");
            return -1;
        }

        imm = (symbol != NULL) ? symbol->vpc : 0xFF;
        break;
    default:
        trace_error(state, "unexpectd rhs type %d for mov\n", rhs->type);
        return -1;
    }

    /* Should we use a wide move? */
    if (imm > SHORT_IMM_MAX) {
        opcode = OPC_WMOV;
        max_bytes = 6;
    }

    if (lhs->reg >= REG_MAX) {
        trace_error(state, "bad lhs register\n");
        return -1;
    }

    cg_emitb(state, opcode);
    cg_emitb(state, lhs->reg);
    while (imm != 0 || byte_count < max_bytes) {
        cg_emitb(state, imm & 0xFF);
        imm >>= 8;
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
        trace_error(state, "lhs of or is not a register\n");
        return -1;
    }

    /* TODO: Support register ORs */
    if (rhs->type != AST_NUMBER) {
        trace_error(state, "rhs of or is not an imm\n");
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

/*
 * Emit machine code for the 'litr' instruction
 *
 * @state; Assembler state
 * @root:  Root node
 */
static int
cg_emit_litr(struct arki_state *state, struct ast_node *root)
{
    if (state == NULL || root == NULL) {
        return -1;
    }

    if (root->type != AST_LITR) {
        trace_error(state, "root is not litr\n");
        return -1;
    }

    if (root->reg >= REG_MAX) {
        trace_error(state, "bad root register for litr\n");
        return -1;
    }

    cg_emitb(state, OPC_LITR);
    cg_emitb(state, root->reg);
    return 0;
}

/*
 * Emit machine code for a store variant instruction
 *
 * @state; Assembler state
 * @root:  Root node
 */
static int
cg_emit_store(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *lhs, *rhs;
    uint8_t opcode;

    if (state == NULL || root == NULL) {
        return -1;
    }

    switch (root->type) {
    case AST_STB:
        opcode = OPC_STB;
        break;
    case AST_STW:
        opcode = OPC_STW;
        break;
    case AST_STL:
        opcode = OPC_STL;
        break;
    case AST_STQ:
        opcode = OPC_STQ;
        break;
    default:
        return -1;
    }

    if ((lhs = root->left) == NULL) {
        trace_error(state, "store has no lhs\n");
        return -1;
    }

    if ((rhs = root->right) == NULL) {
        trace_error(state, "store has no rhs\n");
        return -1;
    }

    if (lhs->type != AST_REG) {
        trace_error(state, "store lhs is not a register\n");
        return -1;
    }

    if (rhs->type != AST_REG) {
        trace_error(state, "store rhs is not a register\n");
        return -1;
    }

    cg_emitb(state, opcode);
    cg_emitb(state, lhs->reg);
    cg_emitb(state, rhs->reg);
    return 0;
}

/*
 * Emit machine code for a load variant instruction
 *
 * @state; Assembler state
 * @root:  Root node
 */
static int
cg_emit_load(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *lhs, *rhs;
    uint8_t opcode;

    if (state == NULL || root == NULL) {
        return -1;
    }

    switch (root->type) {
    case AST_LDB:
        opcode = OPC_LDB;
        break;
    case AST_LDW:
        opcode = OPC_LDW;
        break;
    case AST_LDL:
        opcode = OPC_LDL;
        break;
    case AST_LDQ:
        opcode = OPC_LDQ;
        break;
    default:
        return -1;
    }

    if ((lhs = root->left) == NULL) {
        trace_error(state, "load has no lhs\n");
        return -1;
    }

    if ((rhs = root->right) == NULL) {
        trace_error(state, "load has no rhs\n");
        return -1;
    }

    if (lhs->type != AST_REG) {
        trace_error(state, "load lhs is not a register\n");
        return -1;
    }

    if (rhs->type != AST_REG) {
        trace_error(state, "load rhs is not a register\n");
        return -1;
    }

    cg_emitb(state, opcode);
    cg_emitb(state, lhs->reg);
    cg_emitb(state, rhs->reg);
    return 0;
}

/*
 * Emit bytes to output file
 *
 * @state: Assembler state
 * @root:  Root node
 */
static int
cg_emit_bytes(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *cur;

    if (state == NULL || root == NULL) {
        return -1;
    }

    if (root->type != AST_BYTE) {
        trace_error(state, "emit bytes root not AST_BYTE\n");
        return -1;
    }

    cur = root->right;
    while (cur != NULL) {
        cg_emitb(state, cur->v & 0xFF);
        cur = cur->right;
    }

    return 0;
}

/*
 * Emit a branch
 *
 * @state: Assembler state
 * @root:  Root node
 */
static int
cg_emit_branch(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *rhs;

    if (state == NULL || root == NULL) {
        return -1;
    }

    if (root->type != AST_BRANCH) {
        trace_error(state, "expected 'b' as root node\n");
        return -1;
    }

    rhs = root->right;
    if (rhs->type != AST_REG) {
        trace_error(state, "branch rhs is not register\n");
        return -1;
    }

    cg_emitb(state, OPC_B);
    cg_emitb(state, rhs->reg);
    return 0;
}

/*
 * Emit a skip directive
 *
 * @state: Assembler state
 * @root:  Root node
 */
static int
cg_emit_skip(struct arki_state *state, struct ast_node *root)
{
    struct ast_node *rhs;

    if (state == NULL || root == NULL) {
        return -1;
    }

    if (root->type != AST_SKIP) {
        return -1;
    }

    if ((rhs = root->right) == NULL) {
        trace_error(state, "skip rhs has no number\n");
        return -1;
    }

    /* Skip a number of bytes */
    for (uintptr_t i = 0; i < rhs->v; ++i) {
        cg_emitb(state, 0x00);
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
    case AST_LITR:
        if (cg_emit_litr(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_BYTE:
        if (cg_emit_bytes(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_SKIP:
        if (cg_emit_skip(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_BRANCH:
        if (cg_emit_branch(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_STB:
    case AST_STW:
    case AST_STL:
    case AST_STQ:
        if (cg_emit_store(state, root) < 0) {
            return -1;
        }

        return 0;
    case AST_LDB:
    case AST_LDW:
    case AST_LDL:
    case AST_LDQ:
        if (cg_emit_load(state, root) < 0) {
            return -1;
        }

        return 0;
    default:
        trace_error(state, "bad AST node %d\n", root->type);
        return -1;
    }

    return 0;
}

/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "arki/ast.h"
#include "arki/ptrbox.h"

int
ast_alloc_node(struct arki_state *state, ast_type_t type, struct ast_node **res)
{
    struct ast_node *root;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    root = ptrbox_alloc(&state->ptrbox, sizeof(*root));
    if (root == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    memset(root, 0, sizeof(*root));
    root->type = type;
    *res = root;
    return 0;
}

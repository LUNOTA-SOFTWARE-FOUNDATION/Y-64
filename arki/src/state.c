/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "arki/state.h"

int
arki_state_init(struct arki_state *state, const char *path)
{
    if (state == NULL || path == NULL) {
        errno = -EINVAL;
        return -1;
    }

    memset(state, 0, sizeof(*state));
    if ((state->in_fd = open(path, O_RDONLY)) < 0) {
        return -1;
    }

    state->out_fd = open(DEFAULT_OUT, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (state->out_fd < 0) {
        close(state->in_fd);
        return -1;
    }

    if (ptrbox_init(&state->ptrbox) < 0) {
        close(state->in_fd);
        close(state->out_fd);
        return -1;
    }

    state->line_num = 1;
    return 0;
}

void
arki_state_close(struct arki_state *state)
{
    if (state == NULL) {
        return;
    }

    close(state->in_fd);
    close(state->out_fd);
    ptrbox_destroy(&state->ptrbox);
}

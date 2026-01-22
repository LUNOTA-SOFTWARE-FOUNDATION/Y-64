/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_STATE_H
#define ARKI_STATE_H 1

#include <stdint.h>
#include <stddef.h>
#include "arki/ptrbox.h"
#include "arki/token.h"

/*
 * Represents the assembler state
 *
 * @in_fd:      Input file descriptor
 * @ptrbox:     Global pointer box
 * @last_tok:   Last token
 */
struct arki_state {
    int in_fd;
    struct ptrbox ptrbox;
    struct token last_tok;
};

/*
 * Initialize the assembler state machine
 *
 * @state: Assembler state to initialize
 * @path:  Path of input file
 *
 * Returns zero on success
 */
int arki_state_init(struct arki_state *state, const char *path);

/*
 * Close the assembler state machine
 *
 * @state: State to close
 */
void arki_state_close(struct arki_state *state);

#endif  /* !ARKI_STATE_H  */

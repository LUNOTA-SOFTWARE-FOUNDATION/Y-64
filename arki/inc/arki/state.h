/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_STATE_H
#define ARKI_STATE_H 1

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include "arki/ptrbox.h"
#include "arki/token.h"

/* Default output filename */
#define DEFAULT_OUT "y64.bin"

/*
 * Represents the assembler state
 *
 * @in_fd:      Input file descriptor
 * @out_fd:     Output file descriptor
 * @ptrbox:     Global pointer box
 * @last_tok:   Last token
 * @line_num:   Current line number
 * @pass_count: Number of passes made
 * @origin:     Program origin address
 * @putback:    Putback buffer for lexer
 */
struct arki_state {
    int in_fd;
    int out_fd;
    struct ptrbox ptrbox;
    struct token last_tok;
    size_t line_num;
    size_t pass_count;
    uintptr_t origin;
    char putback;
};

/*
 * Get the value of the current virtual program
 * counter
 *
 * @state: Assembler state
 */
static inline uintptr_t
arki_get_vpc(struct arki_state *state)
{
    size_t offset;

    offset = lseek(state->out_fd, 0, SEEK_CUR);
    return state->origin + offset;
}

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

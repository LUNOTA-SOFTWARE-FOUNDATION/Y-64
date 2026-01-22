/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_LEXER_H
#define ARKI_LEXER_H 1

#include "arki/token.h"
#include "arki/state.h"

/*
 * Scan for a single token from the input file
 *
 * @state: Assembler state machine
 * @res:   Token result is written here
 *
 * Returns zero on success
 */
int lexer_scan(struct arki_state *state, struct token *res);

#endif  /* !ARKI_LEXER_H */

/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_PARSER_H
#define ARKI_PARSER_H 1

#include "arki/state.h"

#define PASS_COUNT 2

/*
 * Begin parsing the input source file
 *
 * Returns zero on success
 */
int arki_parse(struct arki_state *state);

#endif  /* !ARKI_PARSER_H */

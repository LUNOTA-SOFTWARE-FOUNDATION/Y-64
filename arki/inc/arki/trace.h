/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_TRACE_H
#define ARKI_TRACE_H 1

#include <stdio.h>

#define trace_error(state, fmt, ...)        \
    printf("[error]: " fmt, ##__VA_ARGS__); \
    printf("near line %zu\n", (state)->line_num);

#endif  /* !ARKI_TRACE_H */

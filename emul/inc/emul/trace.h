/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_TRACE_H
#define EMUL_TRACE_H 1

#include <stdio.h>

#define trace_error(fmt, ...) \
    printf("[!] " fmt, ##__VA_ARGS__)

#endif  /* !EMUL_TRACE_H */

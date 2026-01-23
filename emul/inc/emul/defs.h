/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_DEFS_H
#define EMUL_DEFS_H 1

/* Compiler attributes */
#define PACKED  __attribute__((packed))

/* Helper macros */
#define NELEM(a) (sizeof(a) / sizeof(a[0]))

#endif  /* !EMUL_DEFS_H */

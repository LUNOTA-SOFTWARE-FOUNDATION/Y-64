/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_MEMCTL_H
#define EMUL_MEMCTL_H 1

#include <stdint.h>
#include <stddef.h>

/*
 * Read n bytes of a memory address into a buffer
 *
 * @addr: Address to read at
 * @buf:  Buffer to read into
 * @n:    Number of bytes to read
 *
 * Returns the number of bytes read on success
 */
ssize_t mem_read(uintptr_t addr, void *buf, size_t n);

/*
 * Write n bytes of buffer into a memory address
 *
 * @addr: Address to write to
 * @buf:  Buffer to read from
 * @n:    Number of bytes to write
 *
 * Returns the number of bytes written on success
 */
ssize_t mem_write(uintptr_t addr, const void *buf, size_t n);

#endif  /* !EMUL_MEMCTL_H */

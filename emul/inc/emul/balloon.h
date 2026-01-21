/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_BALLOON_H
#define EMUL_BALLOON_H

#include <sys/types.h>
#include <stddef.h>

/*
 * Represents memory of some form that is addressable,
 * lazily expandable, etc
 *
 * @buf: Data buffer
 * @cap: Maximum capacity
 * @cur_size: Current size
 */
struct balloon_mem {
    char *buf;
    size_t cap;
    size_t cur_size;
};

/*
 * Allocate a new balloon
 *
 * @res: Balloon result is written here
 * @sz:  Initial size
 * @cap: Maximum capacity of balloon
 *
 * Returns zero on success
 */
int balloon_new(struct balloon_mem *res, size_t sz, size_t cap);

/*
 * Write to a balloon
 *
 * @bp: Balloon pointer
 * @addr: Address to write to
 * @buf: Buffer to write
 * @n: Number of bytes to write
 *
 * Returns the number of bytes written on success,
 * otherwise a less than zero value on failure.
 */
ssize_t balloon_write(
    struct balloon_mem *bp, uintptr_t addr,
    const void *buf, size_t n
);

/*
 * Destroy an allocated balloon
 *
 * @balloon: Balloon to destroy
 */
void balloon_destroy(struct balloon_mem *balloon);

#endif  /* !EMUL_BALLOON_H */

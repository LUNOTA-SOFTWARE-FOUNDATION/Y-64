/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "emul/balloon.h"

int
balloon_new(struct balloon_mem *res, size_t sz, size_t cap)
{
    if (res == NULL || cap == 0) {
        errno = -EINVAL;
        return -1;
    }

    if (sz == 0 || sz == cap) {
        errno = -EINVAL;
        return -1;
    }

    if ((res->buf = malloc(sz)) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    res->cap = cap;
    res->cur_size = sz;
    return 0;
}

ssize_t
balloon_write(struct balloon_mem *bp, uintptr_t addr, const void *buf, size_t n)
{
    void *tmp;

    if (bp == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    if ((addr + n) >= bp->cap) {
        errno = -EIO;
        return -1;
    }

    /* Do we need to resize the buffer? */
    if ((addr + n) >= bp->cur_size) {
        bp->cur_size = (addr + n) + 8;
        if ((tmp = realloc(bp->buf, bp->cur_size)) == NULL) {
            errno = -ENOMEM;
            return -1;
        }

        bp->buf = tmp;
    }

    memcpy(&bp->buf[addr], buf, n);
    return n;
}

ssize_t
balloon_read(struct balloon_mem *bp, uintptr_t addr, void *buf, size_t n)
{
    char *dest;
    size_t pad_len;

    if (bp == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((addr + n) >= bp->cap) {
        errno = -EIO;
        return -1;
    }

    if (addr >= bp->cur_size) {
        memset(buf, 0, n);
        return n;
    }

    /* Does this overlap past the expansion point? */
    if ((addr + n) >= bp->cur_size) {
        dest = (char *)buf;
        memcpy(buf, &bp->buf[addr], bp->cur_size);

        pad_len = (addr + n) - bp->cur_size;
        memset(&dest[bp->cur_size], 0, pad_len);
        return 0;
    }

    memcpy(buf, &bp->buf[addr], n);
    return n;
}

void
balloon_destroy(struct balloon_mem *balloon)
{
    if (balloon == NULL) {
        return;
    }

    free(balloon->buf);
}

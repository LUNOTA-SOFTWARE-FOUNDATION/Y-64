/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stddef.h>
#include <errno.h>
#include "emul/busctl.h"
#include "emul/defs.h"

/* System memory map */
static struct bus_peer_range memmap[] = {
    /* BIOS flash ROM */
    {
        .start = 0x00000000,
        .end   = 0x00100000,
        .peer  = NULL
    },

    /* PD local cache */
    {
        .start = 0x00100000,
        .end   = 0x00101000,
        .peer  = NULL
    },

    /* Chipset registers */
    {
        .start = 0x00110000,
        .end   = 0x00111000,
        .peer = NULL
    },

    /* Main memory */
    {
        .start = 0x116000,
        .end   = -1,
        .peer  = NULL
    }
};

static struct bus_peer_range *
bus_get_range(uintptr_t addr)
{
    struct bus_peer_range *range;
    static size_t n_entries = 0;

    if (n_entries == 0) {
        n_entries = NELEM(memmap);
    }

    for (size_t i = 0; i < n_entries; ++i) {
        range = &memmap[i];
        if (addr >= range->start && addr < range->end) {
            return range;
        }
    }

    return NULL;
}

int
bus_peer_get(struct bus_peer **res, uintptr_t addr)
{
    struct bus_peer_range *range;

    if (res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((range = bus_get_range(addr)) == NULL) {
        errno = -ENODEV;
        return -1;
    }

    *res = range->peer;
    return 0;
}

int
bus_peer_set(struct bus_peer *bp, uintptr_t addr)
{
    struct bus_peer_range *range;

    if (bp == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((range = bus_get_range(addr)) == NULL) {
        errno = -ENODEV;
        return -1;
    }

    if (range->peer != NULL) {
        errno = -EACCES;
        return -1;
    }

    range->peer = bp;
    return 0;
}

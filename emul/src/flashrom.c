/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "emul/flashrom.h"
#include "emul/balloon.h"
#include "emul/busctl.h"

/* Maximum capacity of flash ROM */
#define FLASHROM_CAP 0x100000

/* Actual flash ROM */
static struct balloon_mem flashrom;

/* Bus control operations */
static struct bus_peer flashrom_peer;

static ssize_t
flashrom_read(struct bus_peer *bp, uintptr_t addr, void *buf, size_t n)
{
    if (bp == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    return balloon_read(
        &flashrom,
        bus_peer_mmio(BIOS_FLASHROM_START, addr),
        buf,
        n
    );
}

static int
flashrom_init(void)
{
    if (balloon_new(&flashrom, 8, FLASHROM_CAP) < 0) {
        return -1;
    }

    if (bus_peer_set(&flashrom_peer, BIOS_FLASHROM_START) < 0) {
        return -1;
    }

    return 0;
}

ssize_t
flashrom_flash(const void *buf, size_t n)
{
    if (buf == NULL || n == 0) {
        errno = -EINVAL;
        return -1;
    }

    if (flashrom.buf == NULL) {
        if (flashrom_init() < 0)
            return -1;
    }

    return balloon_write(
        &flashrom,
        0,
        buf,
        n
    );
}

static struct bus_peer flashrom_peer = {
    .type = BUS_PEER_FLASHROM,
    .read = flashrom_read,
    .write = NULL
};

/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "emul/cpu.h"
#include "emul/trace.h"
#include "emul/busctl.h"

/* Local cache peer */
static struct bus_peer lcache_peer;

static ssize_t
lcache_write(struct bus_peer *bp, uintptr_t addr, const void *buf, size_t n)
{
    struct cpu_domain *cpu;

    if ((cpu = bp->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    return balloon_write(
        &cpu->cache,
        bus_peer_mmio(DOMAIN_LCACHE_BASE, addr),
        buf,
        n
    );
}

static ssize_t
lcache_read(struct bus_peer *bp, uintptr_t addr, void *buf, size_t n)
{
    struct cpu_domain *cpu;

    if ((cpu = bp->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    return balloon_read(
        &cpu->cache,
        bus_peer_mmio(DOMAIN_LCACHE_BASE, addr),
        buf,
        n
    );
}

int
cpu_power_up(struct cpu_domain *cpu)
{
    int error;

    if (cpu == NULL) {
        errno = -EINVAL;
        return -1;
    }

    memset(cpu, 0, sizeof(*cpu));
    lcache_peer.data = cpu;
    if (bus_peer_set(&lcache_peer, DOMAIN_LCACHE_BASE) < 0) {
        trace_error("failed to set lcache bus peer\n");
        return -1;
    }

    error = balloon_new(&cpu->cache, 32, DOMAIN_CACHE_SIZE);
    if (error < 0) {
        return -1;
    }

    return 0;
}

void
cpu_destroy(struct cpu_domain *cpu)
{
    if (cpu == NULL) {
        return;
    }

    balloon_destroy(&cpu->cache);
}

static struct bus_peer lcache_peer = {
    .type = BUS_PEER_LCACHE,
    .read = lcache_read,
    .write = lcache_write
};

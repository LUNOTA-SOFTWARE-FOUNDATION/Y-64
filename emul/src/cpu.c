/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include "emul/cpu.h"

int
cpu_power_up(struct cpu_domain *cpu)
{
    int error;

    if (cpu == NULL) {
        errno = -EINVAL;
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

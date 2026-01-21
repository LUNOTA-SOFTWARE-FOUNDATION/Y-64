/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "emul/soc.h"
#include "emul/cpu.h"

int
soc_power_up(struct soc_desc *soc)
{
    if (soc == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (cpu_power_up(&soc->cpu) < 0) {
        return -1;
    }

    return 0;
}

void
soc_destroy(struct soc_desc *soc)
{
    if (soc == NULL) {
        return;
    }

    cpu_destroy(&soc->cpu);
}

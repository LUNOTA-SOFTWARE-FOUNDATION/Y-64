/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_SOC_H
#define EMUL_SOC_H 1

#include <stdint.h>
#include <stddef.h>
#include "emul/cpu.h"

/*
 * Represents a system-on-chip descriptor for the
 * whole SoC
 *
 * @cpu: The main processor
 */
struct soc_desc {
    struct cpu_domain cpu;
};

/*
 * Power up a system on chip
 *
 * @soc: SoC descriptor
 */
int soc_power_up(struct soc_desc *soc);

/*
 * Destroy a system on chip
 *
 * @soc: SoC descriptor
 */
void soc_destroy(struct soc_desc *soc);

#endif  /* !EMUL_SOC_H */

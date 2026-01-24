/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_SOC_H
#define EMUL_SOC_H 1

#include <stdint.h>
#include <stddef.h>
#include "emul/cpu.h"
#include "emul/balloon.h"

#define MAIN_MEMORY_START   0x116000
#define CHIPSET_REGS_START  0x110000
#define DEFAULT_MEM_CAP     0x80000000  /* 2 GiB */

/* Chipset memory control */
#define CS_MEMCTL_CG (1 << 0)   /* Cache gate */

/*
 * Chipset register set
 *
 * @memctl: Memory control registerA
 */
struct chipset_regs {
    uint8_t memctl;
};

/*
 * Represents a system-on-chip descriptor for the
 * whole SoC
 *
 * @cpu:        The main processor
 * @ram:        Random access memory
 * @cs_regs:    Chipset registers
 */
struct soc_desc {
    struct cpu_domain cpu;
    struct balloon_mem ram;
    struct chipset_regs cs_regs;
};

/*
 * Power up a system on chip
 *
 * @soc: SoC descriptor
 */
int soc_power_up(struct soc_desc *soc, size_t memcap);

/*
 * Destroy a system on chip
 *
 * @soc: SoC descriptor
 */
void soc_destroy(struct soc_desc *soc);

#endif  /* !EMUL_SOC_H */

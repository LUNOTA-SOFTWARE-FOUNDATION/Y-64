/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_CPU_H
#define EMUL_CPU_H 1

#include "emul/balloon.h"

#define DOMAIN_CACHE_SIZE 32768

struct cpu_domain {
    struct balloon_mem cache;
};

/*
 * Power-up a processing domain
 */
int cpu_power_up(struct cpu_domain *cpu);

/*
 * Deallocate resources associated with a processing
 * domain
 */
void cpu_destroy(struct cpu_domain *cpu);

#endif  /* !EMUL_CPU_H */

/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_CPU_H
#define EMUL_CPU_H 1

#include "emul/balloon.h"

/* Maximum local cache size */
#define DOMAIN_CACHE_SIZE 32768

/* Address of lcache MMIO */
#define DOMAIN_LCACHE_BASE 0x00100000
#define DOMAIN_LCACHE_SIZE 0x1000

/* Valid opcodes */
#define OPCODE_NOP  0x00        /* No-operation [A] */
#define OPCODE_HLT  0x0D        /* Halt [A] */

/*
 * Register identifiers
 */
typedef enum {
    REG_G0,
    REG_G1,
    REG_G2,
    REG_G3,
    REG_G4,
    REG_G5,
    REG_G6,
    REG_G7,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_TT,
    REG_SP,
    REG_FP,
    REG_PC,
    REG_MAX
} reg_t;

/*
 * Represents an instruction
 *
 * @opcode:  Opcode portion
 * @operand: Operand portion
 * @raw:     Raw value
 */
typedef union {
    struct {
        uint8_t opcode;
        uint64_t operand : 56;
    };
    uint64_t raw;
} inst_t;

/*
 * Represents a processing domain (PD)
 *
 * @domain_id: ID of this PD
 * @cache:     PD local cache
 * @regbank:   Register bank of this PD
 */
struct cpu_domain {
    uint32_t domain_id;
    struct balloon_mem cache;
    uint64_t regbank[REG_MAX];
};

/*
 * Power-up a processing domain
 */
int cpu_power_up(struct cpu_domain *cpu);

/*
 * Dump a processor descriptor
 */
void cpu_dump(struct cpu_domain *cpu);

/*
 * Begin processor execution and let PC tick
 */
void cpu_run(struct cpu_domain *cpu);

/*
 * Deallocate resources associated with a processing
 * domain
 */
void cpu_destroy(struct cpu_domain *cpu);

#endif  /* !EMUL_CPU_H */

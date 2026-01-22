/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_CPU_H
#define EMUL_CPU_H 1

#include "emul/balloon.h"

/* Maximum local cache size */
#define DOMAIN_CACHE_SIZE 65536

/* Address of lcache MMIO */
#define DOMAIN_LCACHE_BASE 0x00100000
#define DOMAIN_LCACHE_SIZE 0x1000

/* Valid opcodes */
#define OPCODE_NOP   0x00        /* No-operation [A] */
#define OPCODE_IMOV  0x01        /* Move wide IMM [C] */
#define OPCODE_IMOVS 0x03        /* Move short IMM [D] */
#define OPCODE_IADD  0x05        /* IMM register ADD [D] */
#define OPCODE_ISUB  0x07        /* IMM register SUB [D] */
#define OPCODE_HLT   0x0D        /* Halt [A] */

/* Error syndrome types */
#define ESR_MAV  0x01           /* Memory access violation */
#define ESR_PV   0x02           /* Protection violation */
#define ESR_UD   0x03           /* Undefined opcode */

/* Interrupt vectors */
#define IVEC_SYNC   0x00          /* Synchronous */
#define IVEC_ASYNC  0x01          /* Asynchronous */

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
 * @itr:       Interrupt table register
 * @esr:       Error syndrome register
 * @n_cycles:  Number of cycles completed
 */
struct cpu_domain {
    uint32_t domain_id;
    struct balloon_mem cache;
    uint64_t regbank[REG_MAX];
    uint64_t itr;
    uint64_t esr;
    size_t n_cycles;
};

/*
 * Power-up a processing domain
 */
int cpu_power_up(struct cpu_domain *cpu);

/*
 * Raise an interrupt on a specific PD
 *
 * @cpu:    PD to raise interrupt on
 * @vector: Interrupt vector to raise
 */
void cpu_raise_int(struct cpu_domain *cpu, uint8_t vector);

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

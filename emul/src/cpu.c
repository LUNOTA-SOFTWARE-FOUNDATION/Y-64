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
#include "emul/memctl.h"

/* Local cache peer */
static struct bus_peer lcache_peer;

/* Register to string lookup table */
static const char *regstr[] = {
    [REG_G0] = "G0",
    [REG_G1] = "G1",
    [REG_G2] = "G2",
    [REG_G3] = "G3",
    [REG_G4] = "G4",
    [REG_G5] = "G5",
    [REG_G6] = "G6",
    [REG_G7] = "G7",
    [REG_A0] = "A0",
    [REG_A1] = "A1",
    [REG_A2] = "A2",
    [REG_A3] = "A3",
    [REG_A4] = "A4",
    [REG_A5] = "A5",
    [REG_A6] = "A6",
    [REG_A7] = "A7",
    [REG_TT] = "TT",
    [REG_SP] = "SP",
    [REG_FP] = "FP",
    [REG_PC] = "PC"
};

/*
 * A table used to lookup masks of valid bits within
 * a special register
 */
#define sreg_index(id) ((id) - 1)
static uint64_t sreg_masktab[] = {
    [sreg_index(SREG_INTCONF)] = 0x1FF
};
#undef sreg_index

/*
 * Read a special register
 *
 * @cpu: PD of special register to read
 * @reg: Special register to read
 *
 * Returns the value within on success, raises a PV# on
 * failure.
 */
static uint64_t
cpu_sreg_read(struct cpu_domain *cpu, sreg_t reg)
{
    if (cpu == NULL) {
        return 0;
    }

    if (reg == SREG_BAD) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return 0;
    }

    return cpu->sreg[reg - 1];
}

/*
 * Write to a special register
 *
 * @cpu: PD of special register to write
 * @reg: Special register to write to
 * @v:   Value to write
 */
static void
cpu_sreg_write(struct cpu_domain *cpu, sreg_t reg, uint64_t v)
{
    if (cpu == NULL) {
        return;
    }

    if (reg == SREG_BAD || reg >= SREG_MAX) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    if ((v & ~sreg_masktab[reg - 1]) != 0) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    cpu->sreg[reg - 1] = v;
}

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

static void
cpu_reset(struct cpu_domain *cpu)
{
    cpu->n_cycles = 0;

    /* Put all registers to their reset state */
    for (int i = 0; i < REG_MAX; ++i) {
        cpu->regbank[i] = (i <= REG_A7)
            ? 0x1A1F1A1F1A1F1A1F
            : 0;
    }

    cpu->sync_vec = 0xFF;
    memset(cpu->sreg, 0, sizeof(cpu->sreg));
}

/*
 * Decode a C-type instruction
 *
 * @cpu:    CPU domain to decode for
 * @inst:   Instruction to decode
 */
static void
cpu_decode_ctype(struct cpu_domain *cpu, inst_t *inst)
{
    reg_t rd;
    uint64_t imm;

    if (cpu == NULL || inst == NULL) {
        return;
    }

    rd = (inst->raw >> 8) & 0xFF;
    imm = (inst->raw >> 16) & 0xFFFFFFFFFFFF;

    /* Is this a valid register? */
    if (rd >= REG_MAX) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    switch (inst->opcode) {
    case OPCODE_IMOV:
        cpu->regbank[rd] = imm;
        break;
    }
}

/*
 * Decode a D-type instruction
 *
 * @cpu:    CPU domain to decode for
 * @inst:   Instruction to decode
 */
static void
cpu_decode_dtype(struct cpu_domain *cpu, inst_t *inst)
{
    reg_t rd;
    uint64_t imm;

    if (cpu == NULL || inst == NULL) {
        return;
    }

    rd = (inst->raw >> 8) & 0xFF;
    imm = (inst->raw >> 16) & 0xFFFF;

    /* Is this a valid register? */
    if (rd >= REG_MAX) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    switch (inst->opcode) {
    case OPCODE_IMOVS:
        cpu->regbank[rd] = imm;
        break;
    case OPCODE_IADD:
        cpu->regbank[rd] += imm;
        break;
    case OPCODE_ISUB:
        cpu->regbank[rd] -= imm;
        break;
    case OPCODE_IOR:
        cpu->regbank[rd] |= imm;
        break;
    }
}

static void
cpu_decode_etype(struct cpu_domain *cpu, inst_t *inst)
{
    reg_t rs;

    if (cpu == NULL || inst == NULL) {
        return;
    }

    rs = (inst->raw >> 8) & 0xFF;

    /* Is the source register valid? */
    if (rs >= REG_MAX) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    switch (inst->opcode) {
    case OPCODE_LITR:
        cpu->itr = cpu->regbank[rs];
        break;
    }
}

/*
 * Read a special register
 *
 * @cpu: Current PD
 */
static void
cpu_srr(struct cpu_domain *cpu)
{
    sreg_t sreg;

    if (cpu == NULL) {
        return;
    }

    sreg = cpu->regbank[REG_G1];
    cpu->regbank[REG_G0] = cpu_sreg_read(cpu, sreg);
}

/*
 * Write a special register
 *
 * @cpu: Current PD
 */
static void
cpu_srw(struct cpu_domain *cpu)
{
    sreg_t sreg;

    if (cpu == NULL) {
        return;
    }

    sreg = cpu->regbank[REG_G1];
    cpu_sreg_write(
        cpu,
        sreg,
        cpu->regbank[REG_G0]
    );
}

/*
 * Service an interrupt vector
 *
 * @cpu: Current PD
 * @vec: Vector to service
 */
static void
cpu_service_vec(struct cpu_domain *cpu, uint8_t vec)
{
    struct ist_entry entry;

    if (cpu == NULL || vec == 0xFF) {
        return;
    }

    printf("[*] got interrupt [vector=%x]\n", vec);
    if (cpu->itr == 0) {
        trace_error("itr invalid - asserting reset...\n");
        cpu_reset(cpu);
        return;
    }

    if (mem_read(cpu->itr, &entry, sizeof(entry)) < 0) {
        cpu->esr = ESR_MAV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    if (entry.p == 0) {
        cpu->esr = ESR_IENP;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    cpu->regbank[REG_PC] = entry.isr;
}

/*
 * Check if a synchronous interrupt is available to
 * be serviced
 *
 * @cpu: Current PD
 */
static void
cpu_poll_sync(struct cpu_domain *cpu)
{
    uint8_t vector;

    if (cpu == NULL) {
        return;
    }

    if ((vector = cpu->sync_vec) != 0xFF) {
        cpu->sync_vec = 0xFF;
        cpu_service_vec(cpu, vector);
    }
}

/*
 * A PD-side wrapper for writing memory
 *
 * @cpu:    Current PD
 * @addr:   Address to write to
 * @buf:    Buffer to write
 * @n:      Number of bytes to write
 */
static ssize_t
cpu_mem_write(struct cpu_domain *cpu, uintptr_t addr, const void *buf, size_t n)
{
    ssize_t count;

    if (cpu == NULL || buf == NULL) {
        return -1;
    }

    if (n == 0) {
        return -1;
    }

    count = mem_write(
        addr,
        buf,
        n
    );

    if (count < 0) {
        cpu->esr = ESR_MAV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return -1;
    }

    return count;
}

/*
 * A PD-side wrapper for reading memory
 *
 * @cpu:    Current PD
 * @addr:   Address to read from
 * @buf:    Buffer to read
 * @n:      Number of bytes to read
 */
static ssize_t
cpu_mem_read(struct cpu_domain *cpu, uintptr_t addr, void *buf, size_t n)
{
    ssize_t count;

    if (cpu == NULL || buf == NULL) {
        return -1;
    }

    if (n == 0) {
        return -1;
    }

    count = mem_read(
        addr,
        buf,
        n
    );

    if (count < 0) {
        cpu->esr = ESR_MAV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return -1;
    }

    return count;
}

/*
 * Decode a B-type instruction
 *
 * @cpu:  Current PD
 * @inst: Instruction to decode
 */
static void
cpu_decode_btype(struct cpu_domain *cpu, inst_t *inst)
{
    reg_t rd, rs;

    if (cpu == NULL || inst == NULL) {
        return;
    }

    /* Extract destination and source regs */
    rd = (inst->raw >> 8) & 0xFF;
    rs = (inst->raw >> 16) & 0xFF;

    if (rd >= REG_MAX || rs >= REG_MAX) {
        cpu->esr = ESR_PV;
        cpu_raise_int(cpu, IVEC_SYNC);
        return;
    }

    switch (inst->opcode) {
    case OPCODE_STB:
        cpu_mem_write(
            cpu,
            cpu->regbank[rd],
            &cpu->regbank[rs],
            1
        );

        break;
    case OPCODE_STW:
        cpu_mem_write(
            cpu,
            cpu->regbank[rd],
            &cpu->regbank[rs],
            2
        );

        break;
    case OPCODE_STL:
        cpu_mem_write(
            cpu,
            cpu->regbank[rd],
            &cpu->regbank[rs],
            4
        );

        break;
    case OPCODE_STQ:
        cpu_mem_write(
            cpu,
            cpu->regbank[rd],
            &cpu->regbank[rs],
            8
        );

        break;
    case OPCODE_LDB:
        cpu_mem_read(
            cpu,
            cpu->regbank[rs],
            &cpu->regbank[rd],
            1
        );

        break;
    case OPCODE_LDW:
        cpu_mem_read(
            cpu,
            cpu->regbank[rs],
            &cpu->regbank[rd],
            2
        );

        break;
    case OPCODE_LDL:
        cpu_mem_read(
            cpu,
            cpu->regbank[rs],
            &cpu->regbank[rd],
            4
        );

        break;
    case OPCODE_LDQ:
        cpu_mem_read(
            cpu,
            cpu->regbank[rs],
            &cpu->regbank[rd],
            8
        );

        break;
    }
}

void
cpu_raise_int(struct cpu_domain *cpu, uint8_t vector)
{
    if (cpu == NULL) {
        return;
    }

    /*
     * The processor does not need to queue up synchronous
     * interrupts as they result from the current instruction.
     */
    if (vector == IVEC_SYNC) {
        cpu->sync_vec = vector;
        return;
    }

    /* TODO: Queue up asynchronous events */
}

void
cpu_dump(struct cpu_domain *cpu)
{
    if (cpu == NULL) {
        return;
    }

    printf("[pd=%d]\n", cpu->domain_id);
    for (int i = 0; i < REG_MAX; ++i) {
        if (i > 0 && (i % 2) == 0) {
            printf("\n");
        }
        printf("%s=0x%016zX ", regstr[i], cpu->regbank[i]);
    }

    printf("\nITR=%016zX\n", cpu->itr);
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

    cpu_reset(cpu);
    return 0;
}

void
cpu_run(struct cpu_domain *cpu)
{
    ssize_t count;
    inst_t inst;

    if (cpu == NULL) {
        return;
    }

    for (;;) {
        count = mem_read(cpu->regbank[REG_PC], &inst, sizeof(inst));
        if (count < 0) {
            trace_error("instruction fetch failure\n");
            return;
        }

        switch (inst.opcode) {
        case OPCODE_NOP:
            cpu->regbank[REG_PC] += 1;
            break;
        case OPCODE_HLT:
            printf("[*] processor halted\n");
            return;
        case OPCODE_SRR:
            cpu_srr(cpu);
            cpu->regbank[REG_PC] += 1;
            break;
        case OPCODE_SRW:
            cpu_srw(cpu);
            cpu->regbank[REG_PC] += 1;
            break;
        case OPCODE_IMOV:
            cpu_decode_ctype(cpu, &inst);
            cpu->regbank[REG_PC] += 8;
            break;
        case OPCODE_STB:
        case OPCODE_STW:
        case OPCODE_STL:
        case OPCODE_STQ:
        case OPCODE_LDB:
        case OPCODE_LDW:
        case OPCODE_LDL:
        case OPCODE_LDQ:
            cpu_decode_btype(cpu, &inst);
            cpu->regbank[REG_PC] += 3;
            break;
        case OPCODE_IADD:
        case OPCODE_IMOVS:
        case OPCODE_ISUB:
        case OPCODE_IOR:
            cpu_decode_dtype(cpu, &inst);
            cpu->regbank[REG_PC] += 4;
            break;
        case OPCODE_LITR:
            cpu_decode_etype(cpu, &inst);
            cpu->regbank[REG_PC] += 2;
            break;
        default:
            cpu->esr = ESR_UD;
            cpu_raise_int(cpu, IVEC_SYNC);
            cpu_poll_sync(cpu);
            continue;
        }

        printf("[*] cycle %zd completed\n", cpu->n_cycles++);
        cpu_dump(cpu);
        cpu_poll_sync(cpu);
    }
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

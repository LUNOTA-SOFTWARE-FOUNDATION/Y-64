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
    /* Put all registers to their reset state */
    for (int i = 0; i < REG_MAX; ++i) {
        cpu->regbank[i] = (i <= REG_A7)
            ? 0x1A1F1A1F1A1F1A1F
            : 0;
    }
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

    printf("\n");
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
    ssize_t count, cycle_count = 0;
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
        default:
            trace_error("undefined opcode %08X\n", inst.opcode);
            return;
        }

        printf("[*] cycle %zd completed\n", cycle_count++);
        cpu_dump(cpu);
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

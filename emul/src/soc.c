/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "emul/defs.h"
#include "emul/soc.h"
#include "emul/cpu.h"
#include "emul/busctl.h"
#include "emul/microsd.h"
#include "emul/memctl.h"
#include "emul/spictl.h"

/* Forward declaration */
static struct bus_peer ram_peer;
static struct bus_peer chipset_peer;

/*
 * Handle SPI transactions
 *
 * @ctl: SPI ctl registers
 */
static int
soc_spi_handle(struct spi_ctl *ctl)
{
    struct spi_prpd prpd;
    ssize_t count;
    int retval = 0;

    if (ctl == NULL) {
        return -1;
    }

    if (ctl->prpd == 0) {
        return -1;
    }

    count = mem_read(
        ctl->prpd,
        &prpd,
        sizeof(prpd)
    );

    if (count < 0) {
        return -1;
    }

    ctl->ctlstat |= SPICTL_BUSY;
    if (prpd.write) {
        retval = spi_write(&prpd);
    }

    ctl->ctlstat &= ~SPICTL_BUSY;
    return (retval < 0) ? -1 : 0;
}

static ssize_t
ram_read(struct bus_peer *peer, uintptr_t addr, void *buf, size_t n)
{
    struct soc_desc *soc;
    struct chipset_regs *cs_regs;

    if (peer == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((soc = peer->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    cs_regs = &soc->cs_regs;
    if (!ISSET(cs_regs->memctl, CS_MEMCTL_CG)) {
        errno = -EIO;
        return -1;
    }

    return balloon_read(
        &soc->ram,
        bus_peer_mmio(MAIN_MEMORY_START, addr),
        buf,
        n
    );
}

static ssize_t
ram_write(struct bus_peer *peer, uintptr_t addr, const void *buf, size_t n)
{
    struct soc_desc *soc;
    struct chipset_regs *cs_regs;

    if (peer == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((soc = peer->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    cs_regs = &soc->cs_regs;
    if (!ISSET(cs_regs->memctl, CS_MEMCTL_CG)) {
        errno = -EIO;
        return -1;
    }

    return balloon_write(
        &soc->ram,
        bus_peer_mmio(MAIN_MEMORY_START, addr),
        buf,
        n
    );
}

static ssize_t
chipset_read(struct bus_peer *peer, uintptr_t addr, void *buf, size_t n)
{
    struct soc_desc *soc;
    struct chipset_regs *cs_regs;

    if (peer == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /* Truncate if needed */
    if (n >= sizeof(*cs_regs)) {
        n = sizeof(*cs_regs);
    }

    if ((soc = peer->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    cs_regs = &soc->cs_regs;
    memcpy(buf, cs_regs, n);
    return n;
}

static ssize_t
chipset_write(struct bus_peer *peer, uintptr_t addr, const void *buf, size_t n)
{
    struct soc_desc *soc;
    struct chipset_regs *cs_regs;
    struct spi_ctl spi_ctl;
    uint8_t memctl;
    char *dest;
    int error = 0;

    if (peer == NULL || buf == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /* Truncate if needed */
    if (n >= sizeof(*cs_regs)) {
        n = sizeof(*cs_regs);
    }

    if ((soc = peer->data) == NULL) {
        errno = -EIO;
        return -1;
    }

    cs_regs = &soc->cs_regs;
    memctl = cs_regs->memctl;
    spi_ctl = cs_regs->spi_ctl;
    dest = (char *)cs_regs;
    memcpy(
        &dest[bus_peer_mmio(CHIPSET_REGS_START, addr)],
        buf,
        n
    );

    /*
     * If the new memctl value does not have the CG bit set,
     * ensure that we are not unsetting it. This bit should
     * be considered sticky and thus unidirectional.
     */
    if (!ISSET(cs_regs->memctl, CS_MEMCTL_CG)) {
        if (ISSET(memctl, CS_MEMCTL_CG))
            cs_regs->memctl |= CS_MEMCTL_CG;
    }

    /* Is there a new SPI transaction? */
    if (spi_ctl.prpd == 0) {
        spi_ctl = cs_regs->spi_ctl;

        if (spi_ctl.prpd != 0)
            error = soc_spi_handle(&cs_regs->spi_ctl);
        if (error != 0)
            return -1;
    }

    return n;
}

int
soc_power_up(struct soc_desc *soc, size_t memcap)
{
    if (soc == NULL) {
        errno = -EINVAL;
        return -1;
    }

    memset(soc, 0, sizeof(*soc));
    if (microsd_init() < 0) {
        return -1;
    }

    if (bus_peer_set(&ram_peer, MAIN_MEMORY_START) < 0) {
        return -1;
    }

    if (bus_peer_set(&chipset_peer, CHIPSET_REGS_START) < 0) {
        return -1;
    }

    if (balloon_new(&soc->ram, 8, memcap) < 0) {
        return -1;
    }

    if (cpu_power_up(&soc->cpu) < 0) {
        balloon_destroy(&soc->ram);
        return -1;
    }

    chipset_peer.data = soc;
    ram_peer.data = soc;
    return 0;
}

void
soc_destroy(struct soc_desc *soc)
{
    if (soc == NULL) {
        return;
    }

    cpu_destroy(&soc->cpu);
    balloon_destroy(&soc->ram);
    microsd_destroy();
}

/* Main memory bus peer */
static struct bus_peer ram_peer = {
    .type = BUS_PEER_RAM,
    .read = ram_read,
    .write = ram_write
};

/* Chipset bus peer */
static struct bus_peer chipset_peer = {
    .type = BUS_PEER_CHIPSET,
    .read = chipset_read,
    .write = chipset_write
};

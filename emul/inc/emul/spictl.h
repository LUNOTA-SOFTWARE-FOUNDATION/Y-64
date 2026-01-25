/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_SPICTL_H
#define EMUL_SPICTL_H 1

#include <sys/queue.h>
#include <sys/types.h>
#include <stdint.h>
#include "emul/defs.h"

/* SPI status bits */
#define SPICTL_BUSY  (1 << 1)

/* SPI device IDs */
#define SPI_MICROSD 0x00

/* SPI block size (must be power-of-two) */
#define SPI_BLOCK_SIZE 16

typedef uint8_t spi_id_t;

/*
 * Represents a single SPI data block that is routed
 * to a specific device.
 *
 * @shift_reg: Data shift register
 * @length:     Shift register length
 * @link: Queue link
 */
struct spi_block {
    uint8_t shift_reg[SPI_BLOCK_SIZE];
    uint8_t length;
    TAILQ_ENTRY(spi_block) link;
};

/*
 * Represents an SPI endpoint device that may have
 * transactions fowarded
 *
 * @id:     Device ID
 * @flush:  Callback to flush block queue
 * @evict:  Evict all entries
 * @blockq: List of blocks to be processed
 */
struct spi_slave {
    spi_id_t id;
    void(*flush)(struct spi_slave *slave, off_t off);
    void(*evict)(struct spi_slave *slave);
    TAILQ_HEAD(, spi_block) blockq;
};

/*
 * Physical region page descriptor used to
 * describe memory areas to the SPI controller
 *
 * @buffer:   Physical address of buffer
 * @length:   Length of buffer
 * @chipsel:  Chip select / ID
 * @write:    If set, operations is a write
 * @offset:   Offset of operation
 */
struct PACKED spi_prpd {
    uintptr_t buffer;
    uint16_t length;
    uint8_t chipsel;
    uint8_t write : 1;
    uint16_t offset;
};

/*
 * Chipset SPI control register
 *
 * @prpd_phys: PRPD physical base address
 * @ctl_stat: Control and status register
 */
struct PACKED chipset_spictl {
    uintptr_t prpd_phys;
    uint8_t ctl_stat;
};

/*
 * Register an SPI device
 *
 * @id:     ID of device to register
 * @device: Device descriptor to register
 *
 * Returns zero on success
 */
int spi_register_device(spi_id_t id, struct spi_slave *device);

/*
 * Send data to an SPI device
 *
 * @prpd: Physical region page descriptor
 *
 * Returns zero on success
 */
int spi_write(struct spi_prpd *prpd);

#endif  /* !EMUL_SPICTL_H */

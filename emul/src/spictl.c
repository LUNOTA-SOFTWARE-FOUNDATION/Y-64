/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include "emul/spictl.h"
#include "emul/defs.h"
#include "emul/memctl.h"

/* List of valid SPI devices */
static struct spi_slave spi_bus[] = {
    [SPI_MICROSD] =
    {
        .id    = SPI_MICROSD,
        .flush = NULL,
        .evict = NULL
    }
};

int
spi_register_device(spi_id_t id, struct spi_slave *device)
{
    struct spi_slave *slvp;

    if (id >= NELEM(spi_bus) || device == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (device->evict == NULL || device->flush == NULL) {
        errno = -EINVAL;
        return -1;
    }

    slvp = &spi_bus[id];
    slvp->flush = device->flush;
    slvp->evict = device->evict;
    TAILQ_INIT(&slvp->blockq);
    return 0;
}

int
spi_write(struct spi_prpd *prpd)
{
    struct spi_block *block = NULL;
    struct spi_slave *slvp;
    uint16_t bytes_left, delta;
    uint8_t id;
    ssize_t count = 0;

    id = prpd->chipsel;
    if (id >= NELEM(spi_bus) || prpd == NULL) {
        errno = -EINVAL;
        return -1;
    }

    bytes_left = prpd->length;
    slvp = &spi_bus[id];

    while (bytes_left > 0) {
        /*
         * Compute the delta / offset of how far we are into
         * the block and allocate a buffer if needed.
         */
        delta = prpd->length - bytes_left;
        if (block == NULL) {
            block = malloc(sizeof(*block));
        }

        if (block == NULL) {
            slvp->evict(slvp);
            errno = -ENOMEM;
            return -1;
        }

        block->length = 0;

        /* Can we read whole chunks? */
        if (bytes_left >= SPI_BLOCK_SIZE) {
            count = mem_read(
                prpd->buffer + delta,
                block->shift_reg,
                SPI_BLOCK_SIZE
            );

            if (count < 0) {
                slvp->evict(slvp);
                free(block);
                errno = -EACCES;
                return -1;
            }

            bytes_left -= SPI_BLOCK_SIZE;
            block->length = SPI_BLOCK_SIZE;

            TAILQ_INSERT_TAIL(&slvp->blockq, block, link);
            block = NULL;
            continue;
        }

        count = mem_read(
            prpd->buffer + delta,
            block->shift_reg,
            bytes_left
        );

        if (count < 0) {
            slvp->evict(slvp);
            free(block);
            errno = -EACCES;
            return -1;
        }

        block->length += bytes_left;
        TAILQ_INSERT_TAIL(&slvp->blockq, block, link);
        block = NULL;
        break;
    }

    /* Flush the device */
    slvp->flush(slvp);
    return 0;
}

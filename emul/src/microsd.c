/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "emul/microsd.h"
#include "emul/spictl.h"
#include "emul/trace.h"

/* Forward declaration */
struct spi_slave microsd_slave;
static bool is_init = false;

static void
microsd_dump_block(struct spi_block *block)
{
    for (uint8_t i = 0; i < block->length; ++i) {
        if (i > 0 && i % 4 == 0) {
            printf("\n");
        }
        printf("%02X ", block->shift_reg[i] & 0xFF);
    }

    printf("\n");
}

static void
microsd_evict(struct spi_slave *slave)
{
    struct spi_block *block;

    block = TAILQ_FIRST(&slave->blockq);
    while (block != NULL) {
        TAILQ_REMOVE(&slave->blockq, block, link);
        free(block);
        block = TAILQ_FIRST(&slave->blockq);
    }
}

static void
microsd_flush(struct spi_slave *slave)
{
    struct spi_block *block;

    printf("begin microsd spi flush\n");
    block = TAILQ_FIRST(&slave->blockq);
    while (block != NULL) {
        microsd_dump_block(block);

        TAILQ_REMOVE(&slave->blockq, block, link);
        free(block);
        block = TAILQ_FIRST(&slave->blockq);
    }
}

int
microsd_init(void)
{
    if (spi_register_device(SPI_MICROSD, &microsd_slave) < 0) {
        trace_error("microsd init failure\n");
        return -1;
    }

    printf("microsd registered\n");
    is_init = true;
    return 0;
}

void
microsd_destroy(void)
{
    if (!is_init) {
        return;
    }

    microsd_evict(&microsd_slave);
}

struct spi_slave microsd_slave = {
    .id = SPI_MICROSD,
    .flush = microsd_flush,
    .evict = microsd_evict
};

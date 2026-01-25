/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "emul/balloon.h"
#include "emul/microsd.h"
#include "emul/spictl.h"
#include "emul/memctl.h"
#include "emul/trace.h"

/* Forward declaration */
struct spi_slave microsd_slave;

static struct balloon_mem sd_data;
static bool is_init = false;

/*
 * Returns true if a vmicro-sd is inserted in the virtual
 * reader.
 */
static inline bool
microsd_is_inserted(void)
{
    return sd_data.buf != NULL;
}

static void
microsd_write_block(struct spi_block *block, off_t offset)
{
    balloon_write(&sd_data, offset, block->shift_reg, block->length);
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
microsd_flush(struct spi_slave *slave, off_t offset)
{
    struct spi_block *block;

    printf("begin microsd spi flush\n");
    block = TAILQ_FIRST(&slave->blockq);

    if (!microsd_is_inserted()) {
        trace_error("flushing to empty microsd port, draining buffers...\n");
    }

    while (block != NULL) {
        if (microsd_is_inserted()) {
            microsd_write_block(block, offset);
        }

        TAILQ_REMOVE(&slave->blockq, block, link);
        free(block);
        block = TAILQ_FIRST(&slave->blockq);
    }
}

static void
microsd_recv(struct spi_slave *slave, struct spi_prpd *prpd)
{
    void *buf;
    ssize_t count;

    if (slave == NULL || prpd == NULL) {
        return;
    }

    if (!microsd_is_inserted()) {
        trace_error("cannot recv, no microsd inserted\n");
        return;
    }

    /* Writes are not allowed */
    if (prpd->write) {
        trace_error("write prpd cannot be used for %s()\n", __func__);
        return;
    }

    if ((buf = malloc(prpd->length)) == NULL) {
        trace_error("microsd buf allocation failure\n");
        return;
    }

    count = balloon_read(&sd_data, prpd->offset, buf, prpd->length);
    if (count < 0) {
        trace_error("microsd read failure\n");
        free(buf);
        return;
    }

    count = mem_write(prpd->buffer, buf, prpd->length);
    if (count < 0) {
        trace_error("microsd read/writeback failure\n");
        free(buf);
        return;
    }

    free(buf);
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

int
microsd_insert(const char *path)
{
    int fd, retval = 0;
    ssize_t fsize, count;
    void *mem;

    if (microsd_is_inserted()) {
        trace_error("microsd already inserted!\n");
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        trace_error("failed to insert '%s' to reader\n", path);
        perror("open");
        return -1;
    }

    /* Grab the file size */
    fsize = lseek(fd, 0, SEEK_END);
    if (fsize < 0) {
        close(fd);
        perror("lseek");
        return -1;
    }

    lseek(fd, 0, SEEK_SET);

    /* Give enough margin for an extra block */
    retval = balloon_new(&sd_data, fsize, fsize + SPI_BLOCK_SIZE);
    if (retval < 0) {
        close(fd);
        perror("balloon_new");
        return -1;
    }

    /* Map the file so we can balloon write */
    mem = mmap(
        NULL,
        fsize,
        PROT_READ,
        MAP_SHARED,
        fd,
        0
    );

    if (mem == NULL) {
        retval = -1;
        perror("mmap");
        goto done;
    }

    count = balloon_write(&sd_data, 0, mem, fsize);
    if (count < 0) {
        retval = -1;
        trace_error("failed to load microsd\n");
        goto done;
    }

    printf("[*] microsd media inserted\n");
done:
    close(fd);
    munmap(mem, fsize);
    return retval;
}

void
microsd_eject(void)
{
    if (!microsd_is_inserted()) {
        return;
    }

    balloon_destroy(&sd_data);
    printf("[*] microsd media ejected\n");
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
    .recv = microsd_recv,
    .flush = microsd_flush,
    .evict = microsd_evict
};

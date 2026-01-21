/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "emul/soc.h"
#include "emul/trace.h"
#include "emul/balloon.h"

#define EMUL_VERSION "0.0.1"

static const char *firmware_path = NULL;

static void
help(void)
{
    printf(
        "Y-64 emulator - beep boop!\n"
        "------------------------------\n"
        "[-h]   Display this help menu\n"
        "[-v]   Display the version\n"
        "[-f]   Firmware ROM file\n"
    );
}

static void
version(void)
{
    printf(
        "Official Y-64 emulator\n"
        "Copyright (c) 2026, Ian Moffett\n"
        "-------------------------------\n"
        "Y-64 emulation version v%s\n",
        EMUL_VERSION
    );
}

static void
cache_dump(struct balloon_mem *cache)
{
    const size_t N_BYTES = 128;
    size_t i;

    printf("[*] dumping first %zd bytes of lcache", N_BYTES);

    for (i = 0; i < N_BYTES; ++i) {
        if (i >= cache->cur_size) {
            printf("\n[?] output truncated to %zd bytes\n", i);
            break;
        }

        if ((i % 16) == 0) {
            printf("\n");
            printf("[%08zX] ", i);
        }

        printf("%02X ", cache->buf[i] & 0xFF);
    }

    if (i >= N_BYTES) {
        printf("\n");
    }
}

static void
emul_run(void)
{
    void *fw_buf;
    struct cpu_domain *cpu;
    struct soc_desc soc;
    size_t fw_size;
    int fw_fd;

    if (soc_power_up(&soc) < 0) {
        trace_error("failed to perform soc power-up\n");
        return;
    }

    cpu = &soc.cpu;
    fw_fd = open(firmware_path, O_RDONLY);

    if (fw_fd < 0) {
        trace_error("failed to open firmware ROM\n");
        perror("open");
        return;
    }

    /* Obtain the size */
    fw_size = lseek(fw_fd, 0, SEEK_END);
    lseek(fw_fd, 0, SEEK_SET);

    if (fw_size >= DOMAIN_CACHE_SIZE) {
        trace_error("fatal: firmware overflow\n");
        close(fw_fd);
        return;
    }

    /* Map the file */
    fw_buf = mmap(
        NULL,
        fw_size,
        PROT_READ,
        MAP_SHARED,
        fw_fd,
        0
    );

    if (fw_buf == NULL) {
        trace_error("failed to open firmware ROM\n");
        perror("mmap");
        return;
    }

    mem_write(DOMAIN_LCACHE_BASE, fw_buf, fw_size);
    cache_dump(&cpu->cache);
    munmap(fw_buf, fw_size);
    soc_destroy(&soc);
}

int
main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hvf:")) != -1) {
        switch (opt) {
        case 'h':
            help();
            return -1;
        case 'v':
            version();
            return -1;
        case 'f':
            firmware_path = strdup(optarg);
            break;
        }
    }

    if (firmware_path == NULL) {
        printf("fatal: expected firmware ROM path!\n");
        help();
        return -1;
    }

    emul_run();
    return 0;
}

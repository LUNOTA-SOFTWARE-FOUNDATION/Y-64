/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "emul/soc.h"
#include "emul/trace.h"
#include "emul/balloon.h"
#include "emul/memctl.h"
#include "emul/flashrom.h"

#define FLASHROM_DUMP_LEN 128
#define EMUL_VERSION "0.0.1"

static const char *firmware_path = NULL;
static size_t ram_cap = DEFAULT_MEM_CAP;

static void
help(void)
{
    printf(
        "Y-64 emulator - beep boop!\n"
        "------------------------------\n"
        "[-h]   Display this help menu\n"
        "[-v]   Display the version\n"
        "[-f]   Firmware ROM file\n"
        "[-r]   Maximum RAM in GiB\n"
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
flashrom_dump(void)
{
    uint8_t buf[FLASHROM_DUMP_LEN];
    size_t i;

    mem_read(BIOS_FLASHROM_START, buf, sizeof(buf));
    printf("[*] dumping first %d bytes of BIOS ROM", FLASHROM_DUMP_LEN);

    for (i = 0; i < FLASHROM_DUMP_LEN; ++i) {
        if ((i % 16) == 0) {
            printf("\n");
            printf("[%08zX] ", i);
        }

        printf("%02X ", buf[i] & 0xFF);
    }

    if (i >= FLASHROM_DUMP_LEN) {
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

    if (soc_power_up(&soc, ram_cap) < 0) {
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
        goto done;
    }

    if (flashrom_flash(fw_buf, fw_size) < 0) {
        trace_error("failed to flash BIOS ROM\n");
        goto done;
    }

    flashrom_dump();
    printf("[*] dumping bootstrap pd state\n");
    cpu_dump(cpu);
    cpu_run(cpu);
done:
    munmap(fw_buf, fw_size);
    close(fw_fd);
    soc_destroy(&soc);
}

int
main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hvf:r:")) != -1) {
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
        case 'r':
            ram_cap = atoi(optarg) * UNIT_GIB;
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

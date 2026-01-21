/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "emul/soc.h"
#include "emul/trace.h"

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
emul_run(void)
{
    struct soc_desc soc;

    if (soc_power_up(&soc) < 0) {
        trace_error("failed to perform soc power-up\n");
        return;
    }

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

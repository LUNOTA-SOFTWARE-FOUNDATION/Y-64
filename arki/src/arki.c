/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <unistd.h>

#define ARKI_VERSION "0.0.1"

static void
help(void)
{
    printf(
        "ARK-I assembler for Y-64\n"
        "Usage: arki <input files>\n"
        "-----------------------------\n"
        "[-h]   Display this help menu\n"
        "[-v]   Display the version\n"
    );
}

static void
version(void)
{
    printf(
        "ARK-I assembler for Y-64\n"
        "Copyright (c) 2026 Ian Moffett\n"
        "------------------------------\n"
        "ARK-I version v%s\n",
        ARKI_VERSION
    );
}

int
main(int argc, char **argv)
{
    int opt;

    if (argc < 2) {
        printf("fatal: expected input file\n");
        help();
    }

    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            help();
            return -1;
        case 'v':
            version();
            return -1;
        }
    }

    return 0;
}

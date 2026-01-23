/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <unistd.h>
#include "arki/state.h"
#include "arki/parser.h"

#define ARKI_VERSION "0.0.2"

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

static int
assemble(const char *path)
{
    struct arki_state state;

    if (arki_state_init(&state, path) < 0) {
        perror("arki_state_init");
        return -1;
    }

    for (int i = 0; i < PASS_COUNT; ++i) {
        if (arki_parse(&state) < 0) {
            return -1;
        }
    }

    arki_state_close(&state);
    return 0;
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

    while (optind < argc) {
        assemble(argv[optind++]);
    }

    return 0;
}

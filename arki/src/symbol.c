/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "arki/symbol.h"

int
symbol_table_init(struct symbol_table *table)
{
    if (table == NULL) {
        errno = -EINVAL;
        return -1;
    }

    table->sym_count = 0;
    TAILQ_INIT(&table->entries);
    return 0;
}

int
symbol_table_new(struct symbol_table *table, const char *name,
    sym_type_t type, struct symbol **res)
{
    struct symbol *sym;

    if (table == NULL || name == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((sym = malloc(sizeof(*sym))) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    memset(sym, 0, sizeof(*sym));
    sym->name = strdup(name);
    sym->type = type;
    sym->id = table->sym_count++;
    TAILQ_INSERT_TAIL(&table->entries, sym, link);

    if (res != NULL) {
        *res = sym;
    }

    return 0;
}

void
symbol_table_destroy(struct symbol_table *table)
{
    struct symbol *sym;

    for (;;) {
        if ((sym = TAILQ_FIRST(&table->entries)) == NULL) {
            break;
        }

        TAILQ_REMOVE(&table->entries, sym, link);
        free(sym->name);
        free(sym);
    }
}

struct symbol *
symbol_by_name(struct symbol_table *table, const char *name)
{
    struct symbol *sym;

    if (table == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(sym, &table->entries, link) {
        if (*sym->name != *name) {
            continue;
        }

        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
    }

    return NULL;
}

struct symbol *
symbol_by_id(struct symbol_table *table, symid_t id)
{
    struct symbol *sym;

    if (table == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(sym, &table->entries, link) {
        if (sym->id == id) {
            return sym;
        }
    }

    return NULL;
}

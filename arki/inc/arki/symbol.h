/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_SYMBOL_H
#define ARKI_SYMBOL_H 1

#include <sys/types.h>
#include <sys/queue.h>
#include <stdint.h>
#include <stddef.h>

/* Symbol ID */
typedef ssize_t symid_t;

/*
 * Represents valid symbol types
 */
typedef enum {
    SYMBOL_NONE,
    SYMBOL_LABEL
} sym_type_t;

/*
 * Represents a program symbol
 *
 * @name: Name of symbol
 * @type: Symbol type
 * @id:   Symbol ID
 * @vpc:  Virtual program counter value for symbol
 * @link: Queue link
 */
struct symbol {
    char *name;
    sym_type_t type;
    symid_t id;
    uintptr_t vpc;
    TAILQ_ENTRY(symbol) link;
};

/*
 * Represents the symbol table
 *
 * @sym_count:  Symbol count
 * @entries:    Actual entries
 */
struct symbol_table {
    size_t sym_count;
    TAILQ_HEAD(, symbol) entries;
};

/*
 * Initialize the program symbol table
 *
 * @table: Table to initialize
 *
 * Returns zero on success
 */
int symbol_table_init(struct symbol_table *table);

/*
 * Allocate a new symbol and place it in the symbol table
 *
 * @table:   Table to add new symbol too
 * @name:    Name of new symbol to create
 * @type:    Type of new symbol to create
 * @res:     Result of new symbol to create
 */
int symbol_table_new(
    struct symbol_table *table, const char *name,
    sym_type_t type, struct symbol **res
);

/*
 * Destroy a specific symbol table
 */
void symbol_table_destroy(struct symbol_table *table);

/*
 * Lookup a symbol by name
 *
 * @table: Table of symbol to lookup
 * @name:  Name of symbol to lookup
 *
 * Returns symbol on success, otherwise a value of NULL
 */
struct symbol *symbol_by_name(struct symbol_table *table, const char *name);

/*
 * Lookup a symbol by name
 *
 * @table: Table of symbol to lookup
 * @id:    ID of symbol to lookup
 *
 * Returns symbol on success, otherwise a value of NULL
 */
struct symbol *symbol_by_id(struct symbol_table *table, symid_t id);

#endif  /* !ARKI_SYMBOL_H */

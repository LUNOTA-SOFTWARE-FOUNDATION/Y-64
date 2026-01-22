/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_FLASHROM_H
#define EMUL_FLASHROM_H 1

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

/* MMIO base and length */
#define BIOS_FLASHROM_START 0x00000000
#define BIOS_FLASHROM_SIZE  0x100000

/*
 * Write an image to firmware flash ROM
 *
 * @buf: Buffer to write
 * @n:   Number of bytes to write
 *
 * Returns the number of bytes written on success,
 * otherwise a less than zero value on failure
 */
ssize_t flashrom_flash(const void *buf, size_t n);

#endif  /* !EMUL_FLASHROM_H */

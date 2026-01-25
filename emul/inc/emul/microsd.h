/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_MICROSD_H
#define EMUL_MICROSD_H 1

/*
 * Initialize the microsd layer
 */
int microsd_init(void);

/*
 * Insert a microsd from a file
 *
 * @path: Path of file to insert
 *
 * Returns zero on success
 */
int microsd_insert(const char *path);

/*
 * Eject the current media from microsd
 */
void microsd_eject(void);

/*
 * Destroy microsd context
 */
void microsd_destroy(void);

#endif  /* !EMUL_MICROSD_H */

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
 * Destroy microsd context
 */
void microsd_destroy(void);

#endif  /* !EMUL_MICROSD_H */

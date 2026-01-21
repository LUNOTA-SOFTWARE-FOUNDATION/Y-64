/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef EMUL_BUSCTL_H
#define EMUL_BUSCTL_H 1

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

#define bus_peer_mmio(range_start, addr_in) \
    ((addr_in) - (range_start))

/*
 * Represents valid bus peer types
 *
 * @BUS_PEER_BAD:       Bad bus peer type
 * @BUS_PEER_LCACHE:    Local cache unit
 */
typedef enum {
    BUS_PEER_BAD,
    BUS_PEER_LCACHE
} bus_peer_t;

/*
 * Represents a range a bus peer exists at
 *
 * @start: Beginning range
 * @end:   End range
 */
struct bus_peer_range {
    uintptr_t start;
    uintptr_t end;
    struct bus_peer *peer;
};

/*
 * Represents a valid bus peer
 *
 * @type:   Bus peer type
 * @range:  Memory range
 * @read:   Read from bus peer
 * @write:  Write to bus peer
 */
struct bus_peer {
    bus_peer_t type;
    struct bus_peer_range range;
    ssize_t(*read)(struct bus_peer *bp, uintptr_t addr, void *buf, size_t n);
    ssize_t(*write)(struct bus_peer *bp, uintptr_t addr, const void *buf, size_t n);
    void *data;
};

/*
 * Obtain a bus peer descriptor from an address range
 *
 * @res:    Bus peer result is written here
 * @addr:   Address to lookup
 *
 * Returns zero on success
 */
int bus_peer_get(struct bus_peer **res, uintptr_t addr);

/*
 * Set a bus peer descriptor to an address range
 *
 * @bp:     Bus peer to write
 * @addr:   Address to set to
 *
 * Returns zero on success
 */
int bus_peer_set(struct bus_peer *bp, uintptr_t addr);

#endif  /* !EMUL_BUSCTL_H */

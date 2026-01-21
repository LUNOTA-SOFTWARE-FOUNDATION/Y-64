/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "emul/trace.h"
#include "emul/busctl.h"
#include "emul/balloon.h"
#include "emul/memctl.h"

ssize_t
mem_read(uintptr_t addr, void *buf, size_t n)
{
    struct bus_peer *peer;

    if (buf == NULL || n == 0) {
        errno = -EINVAL;
        return -1;
    }

    if (bus_peer_get(&peer, addr) < 0) {
        trace_error("failed to get bus peer @ <%zX>\n", addr);
        perror("bus_peer_get");
        return -1;
    }

    if (peer->read == NULL) {
        errno = -EIO;
        return -1;
    }

    return peer->read(
        peer,
        addr,
        buf,
        n
    );
}

ssize_t
mem_write(uintptr_t addr, const void *buf, size_t n)
{
    struct bus_peer *peer;

    if (buf == NULL || n == 0) {
        errno = -EINVAL;
        return -1;
    }

    if (bus_peer_get(&peer, addr) < 0) {
        trace_error("failed to get bus peer @ <%zX>\n", addr);
        perror("bus_peer_get");
        return -1;
    }

    if (peer->write == NULL) {
        errno = -EIO;
        return -1;
    }

    return peer->write(
        peer,
        addr,
        buf,
        n
    );
}

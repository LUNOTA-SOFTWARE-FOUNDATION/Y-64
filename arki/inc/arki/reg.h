/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef ARKI_REG_H
#define ARKI_REG_H 1

#include "arki/token.h"

typedef enum {
    REG_G0,
    REG_G1,
    REG_G2,
    REG_G3,
    REG_G4,
    REG_G5,
    REG_G6,
    REG_G7,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_MAX,
    REG_BAD
} reg_t;

static inline reg_t
token_to_reg(tt_t token)
{
    switch (token) {
    case TT_G0:     return REG_G0;
    case TT_G1:     return REG_G1;
    case TT_G2:     return REG_G2;
    case TT_G3:     return REG_G3;
    case TT_G4:     return REG_G4;
    case TT_G5:     return REG_G5;
    case TT_G6:     return REG_G6;
    case TT_G7:     return REG_G7;
    case TT_A0:     return REG_A0;
    case TT_A1:     return REG_A1;
    case TT_A2:     return REG_A2;
    case TT_A3:     return REG_A3;
    case TT_A4:     return REG_A4;
    case TT_A5:     return REG_A5;
    case TT_A6:     return REG_A6;
    case TT_A7:     return REG_A7;
    default:
        return REG_BAD;
    }

    return REG_BAD;
}

#endif  /* !ARKI_REG_H */

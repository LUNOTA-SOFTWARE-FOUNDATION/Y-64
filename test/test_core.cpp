/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vsoc.h"
#include "Vsoc___024root.h"

#define MAX_SIM_ITER 100

int main(int argc, char** argv, char** env)
{
    Vsoc *soc = new Vsoc;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    soc->trace(m_trace, 5);
    m_trace->open("soc.vcd");

    soc->clk = 0;
    for (int i = 0; i < MAX_SIM_ITER; ++i) {
        soc->eval();
        switch (i) {
        case 0:
            /* Power-up reset */
            soc->reset = 1;
            break;
        case 2:
            /* De-assert reset after two clocks */
            soc->reset = 0;
            break;
        }

        m_trace->dump(i);
        soc->clk ^= 1;
    }

    m_trace->close();
    delete soc;
    return 0;
}

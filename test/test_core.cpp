/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vrbank.h"
#include "Vrbank___024root.h"

#define MAX_SIM_ITER 100

int main(int argc, char** argv, char** env)
{
    Vrbank *rbank = new Vrbank;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    rbank->trace(m_trace, 5);
    m_trace->open("rbank.vcd");

    rbank->clk = 0;
    for (int i = 0; i < MAX_SIM_ITER; ++i) {
        rbank->eval();
        switch (i) {
        case 0:
            /* Power-up reset */
            rbank->reset = 1;
            break;
        case 2:
            /* De-assert reset after two clocks */
            rbank->reset = 0;
            break;
        }

        m_trace->dump(i);
        rbank->clk ^= 1;
    }

    m_trace->close();
    delete rbank;
    return 0;
}

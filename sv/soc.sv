//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// System-on-chip package
//
module soc (
    input wire clk,
    input wire reset
);
    logic [7:0] bus_data_pool;

    domain cpu_domain0 (
        .clk(clk),
        .reset(reset),
        .bus_data_in(bus_data_pool)
    );

    busctl bus_unit (
        .clk(clk),
        .reset(reset),
        .bus_we(0),
        .bus_addr(0),
        .bus_data_in(0),
        .bus_data_out(bus_data_pool)
    );
endmodule

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
    domain cpu_domain0 (
        .clk(clk),
        .reset(reset),
        .pc_inhibit(0)
    );
endmodule

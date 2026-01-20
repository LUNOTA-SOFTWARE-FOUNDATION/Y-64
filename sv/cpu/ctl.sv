//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Processor control unit
//
module ctl #(
    parameter PC_RESET_VAL = 64'h0000000000000000
) (
    input wire clk,
    input wire reset,
    input wire pc_inhibit
);
    logic [63:0] pc;

    //
    // Per every rising edge of the clock, the program
    // counter is to advance if the inhibit line isn't
    // high.
    //
    always @(posedge clk) begin
        if (reset) begin
            pc <= PC_RESET_VAL;
        end
        else if (!pc_inhibit) begin
            pc <= pc + 4;
        end
    end
endmodule

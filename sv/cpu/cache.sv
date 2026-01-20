//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Processor cache unit
//
module cache #(
    parameter CACHE_SIZE = 32768
) (
    /* verilator lint_off UNUSEDSIGNAL */
    input wire [15:0] cache_addr,

    input wire clk,
    input wire cache_we,
    input wire [7:0]  cache_data_in,

    output logic [7:0] cache_data_out
);
    logic [7:0] cbank [0:CACHE_SIZE - 1];
    logic [14:0] index_latch;

    always_ff @(posedge clk) begin
        index_latch <= cache_addr[14:0];
        if (cache_we) begin
            cbank[index_latch] <= cache_data_in;
        end
        else begin
            cache_data_out <= cbank[index_latch];
        end
    end
endmodule

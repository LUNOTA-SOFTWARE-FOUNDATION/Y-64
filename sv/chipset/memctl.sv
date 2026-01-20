//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

module memctl #(
    // 64K RAM for now
    parameter MEM_BYTES = 65536
) (
    input wire clk,
    input logic [7:0] data_in,
    input logic [16:0] addr_in,
    input wire write_en,

    output logic [7:0] data_out
);
    /* verilator lint_off UNUSEDSIGNAL */
    logic [7:0] mbank [0:MEM_BYTES];

    always @(posedge clk) begin
        if (write_en) begin
            mbank[addr_in] <= data_in;
        end
        else begin
            data_out <= mbank[addr_in];
        end
    end
endmodule

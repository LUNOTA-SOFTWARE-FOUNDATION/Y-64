//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Bus control unit
//
module busctl (
    input wire clk,
    input wire reset,

    /* verilator lint_off UNUSEDSIGNAL */
    input wire bus_we,
    input wire [63:0] bus_addr,
    input wire [7:0] bus_data_in,

    output logic [7:0] bus_data_out
);
    /* verilator lint_off UNUSEDSIGNAL */
    logic [15:0] mem_addr_reg;
    logic [7:0]  mem_data_pool;

    memctl mem_ctl (
        .clk(clk),
        .data_in(bus_data_in),
        .addr_in(0),
        .write_en(0),
        .data_out(mem_data_pool)
    );

    always @(posedge clk) begin
        if (reset) begin
            bus_data_out <= 8'h00;
            mem_addr_reg <= 16'h00;
        end
        else begin
            mem_addr_reg <= bus_addr[15:0];
        end
    end
endmodule

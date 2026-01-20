//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Processing domain
//
module domain (
    input wire clk,
    input wire reset
);
    logic reg_bank_we;
    logic [4:0] reg_bank_sel;
    logic [63:0] reg_bank_in;

    /* verilator lint_off UNUSEDSIGNAL */
    logic [63:0] reg_bank_pool;

    rbank reg_bank (
        .clk(clk),
        .reset(reset),
        .reg_we(reg_bank_we),
        .reg_sel(reg_bank_sel),
        .reg_in(reg_bank_in),
        .reg_out(reg_bank_pool)
    );

    always @(posedge clk) begin
        if (reset) begin
            reg_bank_we <= 0;
            reg_bank_sel <= 0;
            reg_bank_pool <= 0;
            reg_bank_in <= 0;
        end
    end
endmodule

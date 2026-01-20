//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Processing domain
//
module domain (
    input wire clk,
    input wire reset,
    input wire pc_inhibit
);
    logic reg_bank_we;
    logic [4:0] reg_bank_sel;
    logic [63:0] reg_bank_in;

    /* verilator lint_off UNUSEDSIGNAL */
    logic [63:0] reg_bank_pool;
    logic [7:0] cache_data_pool;
    logic pc_inhibit_latch;

    rbank reg_bank (
        .clk(clk),
        .reset(reset),
        .reg_we(reg_bank_we),
        .reg_sel(reg_bank_sel),
        .reg_in(reg_bank_in),
        .reg_out(reg_bank_pool)
    );

    cache cache_unit (
        .clk(clk),
        .cache_we(0),
        .cache_data_in(0),
        .cache_data_out(cache_data_pool),
        .cache_addr(0)
    );

    ctl ctl_unit (
        .clk(clk),
        .reset(reset),
        .pc_inhibit(pc_inhibit_latch)
    );

    always @(posedge clk) begin
        if (reset) begin
            pc_inhibit_latch <= 1;
            reg_bank_we <= 0;
            reg_bank_sel <= 0;
            reg_bank_pool <= 0;
            reg_bank_in <= 0;
        end
        else begin
            pc_inhibit_latch <= pc_inhibit;
        end
    end
endmodule

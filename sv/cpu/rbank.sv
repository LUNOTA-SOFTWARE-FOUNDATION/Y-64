//
// Copyright (c) 2026, Ian Moffett.
// Provided under the BSD-3 clause.
//

//
// Processor register bank unit
//
module rbank #(
    parameter REG_RESET_VALUE = 64'h1A1F1A1F1A1F1A1F,
    parameter REG_G0 = 5'h0,
    parameter REG_G1 = 5'h1,
    parameter REG_G2 = 5'h2,
    parameter REG_G3 = 5'h3,
    parameter REG_G4 = 5'h4,
    parameter REG_G5 = 5'h5,
    parameter REG_G6 = 5'h6,
    parameter REG_G7 = 5'h7,

    parameter REG_A0 = 5'h8,
    parameter REG_A1 = 5'h9,
    parameter REG_A2 = 5'hA,
    parameter REG_A3 = 5'hB,
    parameter REG_A4 = 5'hC,
    parameter REG_A5 = 5'hD,
    parameter REG_A6 = 5'hE,
    parameter REG_A7 = 5'hF
) (
    input wire clk,
    input wire reset,
    input wire reg_we,
    input wire [4:0] reg_sel,
    input wire [63:0] reg_in,

    output logic [63:0] reg_out
);
    // General registers
    logic [63:0] g0; logic [63:0] g1;
    logic [63:0] g2; logic [63:0] g3;
    logic [63:0] g4; logic [63:0] g5;
    logic [63:0] g6; logic [63:0] g7;

    // ABI-defined registers
    logic [63:0] a0; logic [63:0] a1;
    logic [63:0] a2; logic [63:0] a3;
    logic [63:0] a4; logic [63:0] a5;
    logic [63:0] a6; logic [63:0] a7;

    always @(posedge clk) begin
        if (reset) begin
            // Reset general registers
            g0 <= REG_RESET_VALUE;
            g1 <= REG_RESET_VALUE;
            g2 <= REG_RESET_VALUE;
            g3 <= REG_RESET_VALUE;
            g4 <= REG_RESET_VALUE;
            g5 <= REG_RESET_VALUE;
            g6 <= REG_RESET_VALUE;
            g7 <= REG_RESET_VALUE;

            // Reset ABI-defined registers
            a0 <= REG_RESET_VALUE;
            a1 <= REG_RESET_VALUE;
            a2 <= REG_RESET_VALUE;
            a3 <= REG_RESET_VALUE;
            a4 <= REG_RESET_VALUE;
            a5 <= REG_RESET_VALUE;
            a6 <= REG_RESET_VALUE;
            a7 <= REG_RESET_VALUE;
        end
        else if (reg_we) begin
            case (reg_sel)
                REG_G0: g0 <= reg_in;
                REG_G1: g1 <= reg_in;
                REG_G2: g2 <= reg_in;
                REG_G3: g3 <= reg_in;
                REG_G4: g4 <= reg_in;
                REG_G5: g5 <= reg_in;
                REG_G6: g6 <= reg_in;
                REG_G7: g7 <= reg_in;
                REG_A0: a0 <= reg_in;
                REG_A1: a1 <= reg_in;
                REG_A2: a2 <= reg_in;
                REG_A3: a3 <= reg_in;
                REG_A4: a4 <= reg_in;
                REG_A5: a5 <= reg_in;
                REG_A6: a6 <= reg_in;
                REG_A7: a7 <= reg_in;
                default: ;
            endcase
        end
        else begin
            case (reg_sel)
                REG_G0: reg_out <= g0;
                REG_G1: reg_out <= g1;
                REG_G2: reg_out <= g2;
                REG_G3: reg_out <= g3;
                REG_G4: reg_out <= g4;
                REG_G5: reg_out <= g5;
                REG_G6: reg_out <= g6;
                REG_G7: reg_out <= g7;
                REG_A0: reg_out <= a0;
                REG_A1: reg_out <= a1;
                REG_A2: reg_out <= a2;
                REG_A3: reg_out <= a3;
                REG_A4: reg_out <= a4;
                REG_A5: reg_out <= a5;
                REG_A6: reg_out <= a6;
                REG_A7: reg_out <= a7;
                default: ;
            endcase
        end
    end
endmodule

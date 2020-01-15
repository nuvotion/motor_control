`include "cypress.v"

module step_dir (
        input  wire clk,
        input  wire step,
        input  wire dir);

    localparam COUNTER_CTRL_ENABLE   = 8'h7; /* Enable Timer               */

    wire clk_int;
    wire [2:0] cs_addr;
    wire [1:0] per_equal;
    wire [1:0] per_zero;
    wire [1:0] per_ff;
    wire [1:0] fifo_nempty;
    wire [1:0] fifo_full;
    wire [7:0] control;
    reg  step_d;
    wire count_enable;
    wire clk_ctrl;
    wire ctrl_enable;
    wire reload;
    wire overflow, underflow;

    cy_psoc3_udb_clock_enable_v1_0 #(.sync_mode(`TRUE))
        clock_enable_block (
            .clock_out(clk_int),
            .clock_in(clk),
            .enable(1'b1)
        );

    assign ctrl_enable = control[COUNTER_CTRL_ENABLE];

    /* Edge Detect on Count input*/
    always @(posedge clk_int) begin
        step_d <= step;
    end
    assign count_enable = (step & !step_d) & ctrl_enable;
    assign overflow = dir & per_equal[1];
    assign underflow = !dir & per_zero[1];
    assign reload = (overflow | underflow) & count_enable;

    /*******************************************************************
     * The clock to operate Control Reg for PSOC3 and PSOC5LP must be 
     * synchronous and run continuosly. In this case the 
     * udb_clock_enable is used only for synchronization. The resulted 
     * clock is always enabled.
     ******************************************************************/
    cy_psoc3_udb_clock_enable_v1_0 #(.sync_mode(`TRUE))
        ctrl_clk_sync (
            .clock_out(clk_ctrl),
            .clock_in(clk),
            .enable(1'b1)
        );

    cy_psoc3_control #(
            .cy_force_order(`TRUE),
            .cy_ctrl_mode_1(8'h00),
            .cy_ctrl_mode_0(8'hFF)
        ) ctrlreg (
            .clock(clk_ctrl),
            .control(control)
        );

    parameter dpconfig0 = {
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG0 Comment:Idle */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC___D0, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG1 Comment:Preload Period (A0 <= D0) */
        `CS_ALU_OP__DEC, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG2 Comment:Dec A0  ( A0 <= A0 - 1 ) */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC___D0, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG3 Comment:Preload Period (A0 <= D0) */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG4 Comment:Idle */
        `CS_ALU_OP__XOR, `CS_SRCA_A0, `CS_SRCB_A0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG5 Comment:Preload Period (A0 <= 0) */
        `CS_ALU_OP__INC, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG6 Comment:Inc A0  ( A0 <= A0 + 1 ) */
        `CS_ALU_OP__XOR, `CS_SRCA_A0, `CS_SRCB_A0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG7 Comment:Preload Period (A0 <= 0) */
          8'hFF, 8'h00,    /* SC_REG4    Comment: */
          8'hFF, 8'hFF, /* SC_REG5    Comment: */
        `SC_CMPB_A0_D1, `SC_CMPA_A0_D1, `SC_CI_B_ARITH,
        `SC_CI_A_ARITH, `SC_C1_MASK_DSBL, `SC_C0_MASK_DSBL,
        `SC_A_MASK_DSBL, `SC_DEF_SI_0, `SC_SI_B_DEFSI,
        `SC_SI_A_DEFSI, /* SC_REG6 Comment: */
        `SC_A0_SRC_ACC, `SC_SHIFT_SL, 1'b0,
        1'b0, `SC_FIFO1_BUS, `SC_FIFO0__A0,
        `SC_MSB_DSBL, `SC_MSB_BIT0, `SC_MSB_NOCHN,
        `SC_FB_NOCHN, `SC_CMP1_NOCHN,
        `SC_CMP0_NOCHN, /* SC_REG7 Comment: */
         10'h00, `SC_FIFO_CLK__DP/*`SC_FIFO_CLK_BUS*/,`SC_FIFO_CAP_FX,
        `SC_FIFO__EDGE,`SC_FIFO__SYNC,`SC_EXTCRC_DSBL,
        `SC_WRK16CAT_DSBL /* SC_REG8 Comment:G8    Comment */
    };

    parameter dpconfig1 = {
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG0 Comment:Idle */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC___D0, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG1 Comment:Preload Period (A0 <= D0) */
        `CS_ALU_OP__DEC, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG2 Comment:Dec A0  ( A0 <= A0 - 1 ) */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC___D0, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG3 Comment:Preload Period (A0 <= D0) */
        `CS_ALU_OP_PASS, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG4 Comment:Idle */
        `CS_ALU_OP__XOR, `CS_SRCA_A0, `CS_SRCB_A0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG5 Comment:Preload Period (A0 <= 0) */
        `CS_ALU_OP__INC, `CS_SRCA_A0, `CS_SRCB_D0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG6 Comment:Inc A0  ( A0 <= A0 + 1 ) */
        `CS_ALU_OP__XOR, `CS_SRCA_A0, `CS_SRCB_A0,
        `CS_SHFT_OP_PASS, `CS_A0_SRC__ALU, `CS_A1_SRC_NONE,
        `CS_FEEDBACK_DSBL, `CS_CI_SEL_CFGA, `CS_SI_SEL_CFGA,
        `CS_CMP_SEL_CFGA, /* CS_REG7 Comment:Preload Period (A0 <= 0) */
          8'hFF, 8'h00, /* SC_REG4    Comment: */
          8'hFF, 8'hFF,    /* SC_REG5    Comment: */
        `SC_CMPB_A0_D1, `SC_CMPA_A0_D1, `SC_CI_B_CHAIN,
        `SC_CI_A_CHAIN, `SC_C1_MASK_DSBL, `SC_C0_MASK_DSBL,
        `SC_A_MASK_DSBL, `SC_DEF_SI_0, `SC_SI_B_DEFSI,
        `SC_SI_A_DEFSI, /* SC_REG6 Comment: */
        `SC_A0_SRC_ACC, `SC_SHIFT_SL, 1'b0,
        1'b0, `SC_FIFO1_BUS, `SC_FIFO0__A0,
        `SC_MSB_DSBL, `SC_MSB_BIT0, `SC_MSB_NOCHN,
        `SC_FB_CHNED, `SC_CMP1_CHNED,
        `SC_CMP0_CHNED, /* SC_REG7 Comment: */
         10'h00, `SC_FIFO_CLK__DP/*`SC_FIFO_CLK_BUS*/,`SC_FIFO_CAP_FX,
        `SC_FIFO__EDGE,`SC_FIFO__SYNC,`SC_EXTCRC_DSBL,
        `SC_WRK16CAT_DSBL
    };

    assign cs_addr = {dir, count_enable, reload};

    cy_psoc3_dp16 #(.cy_dpconfig_a(dpconfig0), .cy_dpconfig_b(dpconfig1))
        counterdp (
            .clk(clk_int),
            .cs_addr(cs_addr),
            .route_si(1'b0),
            .route_ci(1'b0),
            .f0_load(1'b0),
            .f1_load(1'b0),
            .d0_load(1'b0),
            .d1_load(1'b0),
            .ce0(per_equal),
            .z0(per_zero),
            .ff0(per_ff),
            .f0_bus_stat(fifo_nempty),
            .f0_blk_stat(fifo_full)
        );

endmodule


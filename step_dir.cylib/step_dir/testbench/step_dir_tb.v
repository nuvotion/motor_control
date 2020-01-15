`timescale 1ns / 1ns

`include "step_dir.v"

module step_dir_tb();

    localparam CLK_PERIOD = 20;

    initial begin
        $dumpfile(``WAVE_FILE);
        $dumpvars;
        clk = 0;
        #(40_000 * CLK_PERIOD) $finish;
    end

    wire step;
    reg clk, step_en, dir;
    reg [15:0] sim_counter;

    step_dir step_dir (
        .clk(clk),
        .step(step),
        .dir(dir)
    );

    always #CLK_PERIOD clk <= !clk;

    always @(clk) step_dir.ctrlreg.cpu_clock <= clk;
    always @(clk) step_dir.counterdp.U0.cpu_clock <= clk;
    always @(clk) step_dir.counterdp.U1.cpu_clock <= clk;
    
    initial begin
        dir = 1;
        step_en = 0;
        sim_counter = 0;

        step_dir.counterdp.U0.d0_write(1999 & 8'hFF);
        step_dir.counterdp.U1.d0_write(1999 >> 8);
        step_dir.ctrlreg.control_write(1 << 7);

        wait (sim_counter == 100);
        step_en = 1;
    end

    always @(posedge clk) begin
        sim_counter <= sim_counter + 1;
    end

    assign step = step_en & sim_counter[1]; 
 
endmodule

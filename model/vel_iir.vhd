library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.fixed.all;

entity vel_iir is
    port (
        clk : in  std_logic;
        rst : in  std_logic;

        pos_in      : in  signed(31 downto 0);
        torque_in   : in  signed(31 downto 0)
    );
end entity vel_iir;

architecture float_iir of vel_iir is

    constant C_F_PERIOD : real := 0.0002;

    constant C_D        : real := 0.9;
    constant C_W        : real := 1000.0;

    constant C_B_P0 : real := 2.0 * C_D * C_W * C_F_PERIOD;
    constant C_B_P1 : real := C_W**2 * C_F_PERIOD**2 - C_B_P0;
    constant C_B_V0 : real := C_W**2 * C_F_PERIOD;
    constant C_B_V1 : real := -C_B_V0;
    constant C_A_1  : real := 2.0 - C_W**2 * C_F_PERIOD**2 - C_B_P0;
    constant C_A_2  : real := C_B_P0 - 1.0;

    signal r_pos_in     : real := 0.0;
    signal r_pos_in_d   : real := 0.0;
    signal r_pos_out    : real := 0.0;
    signal r_pos_out_d1 : real := 0.0;
    signal r_pos_out_d2 : real := 0.0;
    signal r_vel        : real := 0.0;
    signal r_vel_d1     : real := 0.0;
    signal r_vel_d2     : real := 0.0;

begin

    r_pos_in <= to_real(pos_in);

    r_pos_out <= r_pos_in       * C_B_P0 + 
                 r_pos_in_d     * C_B_P1 +
                 r_pos_out_d1   * C_A_1  +
                 r_pos_out_d2   * C_A_2;

    r_vel     <= r_pos_in       * C_B_V0 + 
                 r_pos_in_d     * C_B_V1 +
                 r_vel_d1       * C_A_1  +
                 r_vel_d2       * C_A_2;

    mem_p : process(clk) begin
        if rising_edge(clk) then
            r_pos_in_d      <= r_pos_in;
            r_pos_out_d1    <= r_pos_out;
            r_pos_out_d2    <= r_pos_out_d1;
            r_vel_d1        <= r_vel;
            r_vel_d2        <= r_vel_d1;
        end if;
    end process mem_p;

end architecture float_iir;

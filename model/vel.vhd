library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.fixed.all;

entity vel is
    port (
        clk : in  std_logic;
        rst : in  std_logic;

        pos_in      : in  signed(31 downto 0);
        torque_in   : in  signed(31 downto 0)
    );
end entity vel;

architecture fixed of vel is

    constant C_F_PERIOD : real := 0.0002;
    constant C_PERIOD   : signed(31 downto 0) := to_accum(C_F_PERIOD);
    constant C_LP       : signed(31 downto 0) := to_accum(
                            1.0 / (1.0 / (50.0*2.0*C_F_PI*C_F_PERIOD) + 1.0));
    constant C_ACC_GAIN : signed(31 downto 0) := to_accum(
                            1000.0*1000.0*C_F_PERIOD);
    constant C_VEL_GAIN : signed(31 downto 0) := to_accum(
                            2.0*0.9*1000.0*C_F_PERIOD);

    shared variable v_vel_sum       : signed(31 downto 0) := (others => '0');
    shared variable v_acc_sum       : signed(31 downto 0) := (others => '0');
    shared variable v_pos_error     : signed(31 downto 0) := (others => '0');
    shared variable v_acc           : signed(31 downto 0) := (others => '0');
    shared variable v_acc_ff        : signed(31 downto 0) := (others => '0');
    shared variable v_vel           : signed(31 downto 0) := (others => '0');
    shared variable v_pos_out       : signed(31 downto 0) := (others => '0');
    shared variable v_last_acc      : signed(31 downto 0) := (others => '0');

    signal r_vel_sum                : real;
    signal r_acc_sum                : real;
    signal r_pos_error              : real;
    signal r_acc                    : real;
    signal r_acc_ff                 : real;
    signal r_vel                    : real;
    signal r_pos_out                : real;
    signal r_last_acc               : real;

    signal s_vel_sum                : signed(31 downto 0);
    signal s_acc_sum                : signed(31 downto 0);
    signal s_pos_error              : signed(31 downto 0);
    signal s_acc                    : signed(31 downto 0);
    signal s_acc_ff                 : signed(31 downto 0);
    signal s_vel                    : signed(31 downto 0);
    signal s_pos_out                : signed(31 downto 0);
    signal s_last_acc               : signed(31 downto 0);

begin

    vars_p : process(clk) begin
        r_vel_sum   <= to_real(v_vel_sum);
        r_acc_sum   <= to_real(v_acc_sum);
        r_pos_error <= to_real(v_pos_error);
        r_acc       <= to_real(v_acc);
        r_acc_ff    <= to_real(v_acc_ff);
        r_vel       <= to_real(v_vel);
        r_pos_out   <= to_real(v_pos_out);
        r_last_acc  <= to_real(v_last_acc);

        s_vel_sum   <= v_vel_sum;
        s_acc_sum   <= v_acc_sum;
        s_pos_error <= v_pos_error;
        s_acc       <= v_acc;
        s_acc_ff    <= v_acc_ff;
        s_vel       <= v_vel;
        s_pos_out   <= v_pos_out;
        s_last_acc  <= v_last_acc;
    end process vars_p;

    vel_p : process(clk) begin
        if rising_edge(clk) then
            v_vel_sum       := v_vel_sum + accum_mul(v_acc_sum, C_PERIOD);
            v_pos_error     := accum_minus(pos_in, v_vel_sum);
            v_acc           := accum_mul(torque_in, C_PERIOD);
            v_last_acc      := accum_mul(v_acc, C_LP) +
                               accum_mul(v_last_acc, C_1K - C_LP);
            v_acc_ff        := v_acc - v_last_acc;
            v_acc_ff        := v_acc_ff + accum_mul(v_pos_error, C_ACC_GAIN);
            v_acc_sum       := v_acc_sum + v_acc_ff;
            v_vel           := v_acc_sum;
            v_vel_sum       := v_vel_sum + accum_mul(v_pos_error, C_VEL_GAIN);
            v_vel_sum       := accum_mod(v_vel_sum);
            v_pos_out       := v_vel_sum;
        end if;
    end process vel_p;

end architecture fixed;

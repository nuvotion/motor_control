library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

use work.fixed.all;

entity vel_double is
    port (
        clk : in  std_logic;
        rst : in  std_logic;

        pos_in      : in  signed(31 downto 0);
        torque_in   : in  signed(31 downto 0);

        p_pos_out   : out signed(31 downto 0);
        p_vel       : out signed(31 downto 0)
    );
end entity vel_double;

architecture fixed of vel_double is

    constant C_F_PERIOD : real := 0.0002;
    constant C_PERIOD   : signed(31 downto 0) := to_accum(C_F_PERIOD);
    constant C_LP       : signed(31 downto 0) := to_accum(
                            1.0 - exp(-2.0*C_F_PI*C_F_PERIOD*50.0*0.0));
    constant C_ACC_GAIN : signed(31 downto 0) := to_accum(
                            1000.0*1000.0*C_F_PERIOD);
    constant C_VEL_GAIN : signed(31 downto 0) := to_accum(
                            2.0*0.9*1000.0*C_F_PERIOD);

    shared variable v_vel_sum       : signed(63 downto 0) := (others => '0');
    shared variable v_acc_sum       : signed(63 downto 0) := (others => '0');
    shared variable v_pos_error     : signed(31 downto 0) := (others => '0');
    shared variable v_acc_delta     : signed(31 downto 0) := (others => '0');
    shared variable v_acc_ff        : signed(31 downto 0) := (others => '0');
    shared variable v_vel           : signed(31 downto 0) := (others => '0');
    shared variable v_pos_out       : signed(31 downto 0) := (others => '0');
    shared variable v_last_acc      : signed(31 downto 0) := (others => '0');

    signal r_vel_sum                : real;
    signal r_acc_sum                : real;
    signal r_pos_error              : real;
    signal r_acc_delta              : real;
    signal r_acc_ff                 : real;
    signal r_vel                    : real;
    signal r_pos_out                : real;
    signal r_last_acc               : real;

    signal s_vel_sum                : signed(63 downto 0);
    signal s_acc_sum                : signed(63 downto 0);
    signal s_pos_error              : signed(31 downto 0);
    signal s_acc_delta              : signed(31 downto 0);
    signal s_acc_ff                 : signed(31 downto 0);
    signal s_vel                    : signed(31 downto 0);
    signal s_pos_out                : signed(31 downto 0);
    signal s_last_acc               : signed(31 downto 0);

begin

    vars_p : process(clk) begin
        r_vel_sum   <= to_real(read_daccum(v_vel_sum));
        r_acc_sum   <= to_real(read_daccum(v_acc_sum));
        r_pos_error <= to_real(v_pos_error);
        r_acc_delta <= to_real(v_acc_delta);
        r_acc_ff    <= to_real(v_acc_ff);
        r_vel       <= to_real(v_vel);
        r_pos_out   <= to_real(v_pos_out);
        r_last_acc  <= to_real(v_last_acc);

        s_vel_sum   <= v_vel_sum;
        s_acc_sum   <= v_acc_sum;
        s_pos_error <= v_pos_error;
        s_acc_delta <= v_acc_delta;
        s_acc_ff    <= v_acc_ff;
        p_vel       <= v_vel;
        p_pos_out   <= v_pos_out;
        s_last_acc  <= v_last_acc;
    end process vars_p;

    vel_p : process(clk) begin
        if rising_edge(clk) then
            v_pos_error := accum_minus(pos_in, read_daccum(v_vel_sum));
            v_acc_delta := torque_in - v_last_acc;
            v_vel_sum   := daccum_mac(read_daccum(v_acc_sum), C_PERIOD, v_vel_sum);
            v_last_acc  := v_last_acc + accum_mul(v_acc_delta, C_LP);
            v_acc_sum   := daccum_mac(v_acc_delta, C_PERIOD, v_acc_sum);
            v_acc_sum   := daccum_mac(v_pos_error, C_ACC_GAIN, v_acc_sum);
            v_vel_sum   := daccum_mac(v_pos_error, C_VEL_GAIN, v_vel_sum);
            v_vel_sum   := daccum_mod(v_vel_sum);

            v_pos_out   := read_daccum(v_vel_sum);
            v_vel       := read_daccum(v_acc_sum);
        end if;
    end process vel_p;

end architecture fixed;

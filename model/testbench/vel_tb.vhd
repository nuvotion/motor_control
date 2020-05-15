-- ghdl_flags: --stop-time=1us --wave=vel_tb.ghw

-- ghdl_deps: fixed.vhd
-- ghdl_deps: vel.vhd
-- ghdl_deps: vel_iir.vhd
-- ghdl_deps: vel_double.vhd

use std.textio.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

use work.fixed.all;

entity vel_tb is begin
end entity vel_tb;

architecture tb of vel_tb is

    signal clk : std_logic := '1';
    signal rst : std_logic := '1';
    signal dir : std_logic := '1';

    signal dbg_incr         : signed(31 downto 0) := (others => '0');
    signal dbg_angle        : signed(31 downto 0) := (others => '0');
    signal dbg_torque       : signed(31 downto 0) := (others => '0');
    signal pos_in           : signed(31 downto 0) := (others => '0');

    signal pos_out          : signed(31 downto 0);
    signal pos_out_no_ff    : signed(31 downto 0);

    signal pos_in_real      : real;
    signal torque_in_real   : real;

    signal pos_err          : real;
    signal pos_err_no_ff    : real;

begin

    clk <= not clk after 0.5 ns;

    reset_p : process begin
        wait for 2 ns;
        wait until rising_edge(clk);
        rst <= '0';
        dir <= '1';
        wait for 200 ns;
        wait until rising_edge(clk);
        dir <= '0';
        wait;
    end process reset_p;

    stim_p : process(clk) begin
        if rising_edge(clk) then
            if rst = '0' then
                if dir = '1' then
                    dbg_incr    <= dbg_incr + to_accum(0.0002);
                    dbg_angle   <= dbg_angle + dbg_incr;
                    dbg_torque  <= to_accum(4000.0);
                else
                    dbg_incr    <= dbg_incr - to_accum(0.0002);
                    dbg_angle   <= dbg_angle + dbg_incr;
                    dbg_torque  <= to_accum(-4000.0);
                end if;
            end if;
        end if;
    end process stim_p;

    pos_in          <= accum_mod(dbg_angle);
    pos_in_real     <= to_real(pos_in);
    torque_in_real  <= to_real(dbg_torque);

--    u_vel : entity work.vel
--        port map (
--            clk => clk,
--            rst => rst,
--
--            pos_in      => pos_in,
--            torque_in   => (others => '0')
--        );
--
--    u_vel_iir : entity work.vel_iir
--        port map (
--            clk => clk,
--            rst => rst,
--
--            pos_in      => pos_in,
--            torque_in   => (others => '0')
--        );

    u_vel_double : entity work.vel_double
        port map (
            clk => clk,
            rst => rst,

            pos_in      => pos_out, --pos_in,
            torque_in   => dbg_torque,

            p_pos_out   => pos_out
        );

    u_no_ff : entity work.vel_double
        port map (
            clk => clk,
            rst => rst,

            pos_in      => pos_in,
            torque_in   => (others => '0'),

            p_pos_out   => pos_out_no_ff
        );

    pos_err         <= to_real(pos_out - pos_in);
    pos_err_no_ff   <= to_real(pos_out_no_ff - pos_in);

end architecture tb;

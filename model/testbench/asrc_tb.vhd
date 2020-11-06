-- ghdl_flags: --stop-time=4000ms --wave=asrc_tb.ghw

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

entity asrc_tb is begin
end entity asrc_tb;

architecture tb of asrc_tb is

    signal clk_g2 : std_logic := '1';
    signal clk_mc : std_logic := '1';

    shared variable g2_sample_cnt       : signed(31 downto 0) := (others => '0');
    shared variable g2_sample_cnt_mc    : signed(31 downto 0) := (others => '0');
    shared variable g2_sample_diff      : signed(31 downto 0);

    type sample_buf_t is array (natural range <>) of signed(31 downto 0);
    shared variable sample_buf  : sample_buf_t(0 to 7) := (others => (others => '0'));

    shared variable g2_accum            : signed(31 downto 0) := to_accum(0.0);
    shared variable sample_ptr          : unsigned(2 downto 0) := "011";
    shared variable wr_ptr              : unsigned(2 downto 0) := "000";
    shared variable rd_ptr_a            : unsigned(2 downto 0);
    shared variable rd_ptr_b            : unsigned(2 downto 0);
    shared variable ratio               : unsigned(31 downto 0) := X"00C49BA5"; -- 0.006 * 2^31
    shared variable phase_overflow      : std_logic;
    shared variable phase               : unsigned(31 downto 0) := (others => '0');
    shared variable sample              : signed(31 downto 0);
    shared variable sample_d            : signed(31 downto 0);
    shared variable sample_diff         : signed(31 downto 0);

    signal s_sample_ptr         : unsigned(2 downto 0);
    signal s_wr_ptr             : unsigned(2 downto 0);
    signal s_rd_ptr_a           : unsigned(2 downto 0);
    signal s_rd_ptr_b           : unsigned(2 downto 0);

    signal r_ratio              : real;
    signal r_phase              : real;
    signal r_sample             : real;
    signal r_sample_diff        : real;

    type r_sample_buf_t is array (natural range <>) of real;
    signal r_sample_buf         : r_sample_buf_t(0 to 7);

begin

    clk_g2 <= not clk_g2 after 99.50 us;
    clk_mc <= not clk_mc after 100 us;

    g2_p : process(clk_g2) begin
        if rising_edge(clk_g2) then
            g2_sample_cnt := g2_sample_cnt + 1;
        end if;
    end process g2_p;

    mc_p : process(clk_mc) begin
        if rising_edge(clk_mc) then
            -- Circular buffer --------------------------------------------------------------------
            g2_sample_diff      := g2_sample_cnt - g2_sample_cnt_mc;
            g2_sample_cnt_mc    := g2_sample_cnt; 

            g2_accum := g2_accum + to_accum(16.0);
            sample_buf(to_integer(wr_ptr)) := g2_accum;
            wr_ptr := wr_ptr + 1;
            if g2_sample_diff = 2 then
                g2_accum := g2_accum + to_accum(16.0);
                sample_buf(to_integer(wr_ptr)) := g2_accum;
                wr_ptr := wr_ptr + 1;
            end if;

            -- Phase accumulator ------------------------------------------------------------------
            phase           := phase + ratio; 
            phase_overflow  := phase(31);
            phase           := phase and X"7FFF_FFFF";

            if g2_sample_diff = 2 and phase_overflow = '0' and sample_ptr > 0 then
                sample_ptr := sample_ptr - 1;
            elsif g2_sample_diff = 1 and phase_overflow = '1' and sample_ptr < 6 then 
                sample_ptr := sample_ptr + 1;
            end if;

            -- Ratio estimation -------------------------------------------------------------------
            case sample_ptr is
                when "000" =>
                    ratio := ratio + 2048;
                when "001" =>
                    ratio := ratio + 1024;
                when "010" =>
                    ratio := ratio + 512;
                when "100" =>
                    ratio := ratio - 512;
                when "101" =>
                    ratio := ratio - 1024;
                when "110" =>
                    ratio := ratio - 2048;

                when others =>
            end case;

            -- Interpolator -----------------------------------------------------------------------
            rd_ptr_a := wr_ptr + sample_ptr;
            rd_ptr_b := wr_ptr + sample_ptr + 1;

            sample := sample_buf(to_integer(rd_ptr_a)) + accum_mul(signed(X"0000" & phase(31 downto 16)),
                                sample_buf(to_integer(rd_ptr_b)) - sample_buf(to_integer(rd_ptr_a)));

            -- Debug ------------------------------------------------------------------------------
            s_sample_ptr    <= sample_ptr;
            s_wr_ptr        <= wr_ptr;
            s_rd_ptr_a      <= rd_ptr_a;
            s_rd_ptr_b      <= rd_ptr_b;

            sample_diff     := sample - sample_d;
            sample_d        := sample;

            r_ratio         <= real(to_integer(ratio)) / 2.0**31;
            r_phase         <= real(to_integer(phase)) / 2.0**31;
            r_sample        <= to_real(sample);
            r_sample_diff   <= to_real(sample_diff);

            for i in 0 to 7 loop
                r_sample_buf(i) <= to_real(sample_buf(i));
            end loop;
        end if;
    end process mc_p;

end architecture tb;

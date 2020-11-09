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

    constant C_ENCODER_GAIN : signed(31 downto 0) := to_accum(C_F_PI / 1000.0);
    constant C_DEPTH_BITS   : natural := 4;
    constant C_DEPTH        : natural := 2**C_DEPTH_BITS;

    signal clk_g2 : std_logic := '1';
    signal clk_mc : std_logic := '1';

    shared variable g2_sample_cnt       : unsigned(31 downto 0) := (others => '0');
    shared variable g2_sample_cnt_mc    : unsigned(31 downto 0) := (others => '0');
    shared variable g2_sample_diff      : unsigned(31 downto 0);

    type sample_buf_t is array (natural range <>) of signed(31 downto 0);
    shared variable sample_buf  : sample_buf_t(0 to C_DEPTH-1) := (others => (others => '0'));

    shared variable sample_time         : real := 0.0;
    shared variable sample_vel          : real := 0.0;
    shared variable sample_pos          : real := 0.0;
    shared variable sample_pos_int      : signed(31 downto 0);
    shared variable sample_ptr          : unsigned(C_DEPTH_BITS-1 downto 0) := "0111";
    shared variable wr_ptr              : unsigned(C_DEPTH_BITS-1 downto 0) := "0000";
    shared variable rd_ptr_a            : unsigned(C_DEPTH_BITS-1 downto 0);
    shared variable rd_ptr_b            : unsigned(C_DEPTH_BITS-1 downto 0);
    shared variable ratio_num           : unsigned(31 downto 0) := to_unsigned(5029, 32);
    shared variable ratio_den           : unsigned(31 downto 0) := to_unsigned(5000, 32);
    shared variable ratio_div           : unsigned(31 downto 0);
    shared variable ratio_mul           : unsigned(63 downto 0);
    shared variable ratio               : signed(31 downto 0) := X"00BE0DED"; -- 0.006 * 2^31
    shared variable phase_overflow      : std_logic;
    shared variable phase               : signed(31 downto 0) := (others => '0');
    shared variable sample_sub          : signed(31 downto 0);
    shared variable sample_mul          : signed(63 downto 0);
    shared variable sample              : signed(31 downto 0);
    shared variable sample_mod          : signed(31 downto 0);
    shared variable sample_d            : signed(31 downto 0);
    shared variable sample_diff         : signed(31 downto 0);
    shared variable sample_mod_d        : signed(31 downto 0);
    shared variable sample_mod_diff     : signed(31 downto 0);

    signal pos_in           : signed(31 downto 0) := (others => '0');
    signal pos_out          : signed(31 downto 0);

    signal s_sample_ptr         : unsigned(C_DEPTH_BITS-1 downto 0);
    signal s_wr_ptr             : unsigned(C_DEPTH_BITS-1 downto 0);
    signal s_rd_ptr_a           : unsigned(C_DEPTH_BITS-1 downto 0);
    signal s_rd_ptr_b           : unsigned(C_DEPTH_BITS-1 downto 0);
    signal s_ratio_num          : unsigned(31 downto 0);
    signal s_ratio_den          : unsigned(31 downto 0);
    signal s_ratio_div          : unsigned(31 downto 0);

    signal r_sample_vel         : real := 0.0;
    signal r_ratio              : real := 0.0;
    signal r_phase              : real := 0.0;
    signal r_sample             : real := 0.0;
    signal r_sample_diff        : real := 0.0;
    signal r_sample_mod_diff    : real := 0.0;
    signal r_sample_mod         : real := 0.0;
    signal r_sample_sub         : real := 0.0;

    type r_sample_buf_t is array (natural range <>) of real;
    signal r_sample_buf         : r_sample_buf_t(0 to C_DEPTH-1);

    function motion_profile(t : real) return real is begin
        if t > 2.0 then
            return 8.0 * (tanh(-4.0*(t - 3.0)) + 1.0);
        else
            return 8.0 * (tanh(4.0*(t - 1.0)) + 1.0);
        end if;
    end function motion_profile;

begin

    clk_g2 <= not clk_g2 after 99.42 us;
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

            sample_time         := sample_time + 200.0e-6;
            sample_vel          := motion_profile(sample_time);
            sample_pos          := sample_pos + sample_vel;
            sample_pos_int      := to_signed(integer(sample_pos) mod 2000, 32);
            sample_buf(to_integer(wr_ptr)) := sample_pos_int;
            wr_ptr := wr_ptr + 1;
            if g2_sample_diff = 2 then
                sample_time         := sample_time + 200.0e-6;
                sample_vel          := motion_profile(sample_time);
                sample_pos          := sample_pos + sample_vel;
                sample_pos_int      := to_signed(integer(sample_pos) mod 2000, 32);
                sample_buf(to_integer(wr_ptr)) := sample_pos_int;
                wr_ptr := wr_ptr + 1;
            end if;

            -- Ratio estimation -------------------------------------------------------------------
            if ratio_num(14) = '1' then
                ratio_num := '0' & ratio_num(31 downto 1);
                ratio_den := '0' & ratio_den(31 downto 1);
            end if;

            ratio_num := ratio_num + g2_sample_diff;
            ratio_den := ratio_den + 1;

            ratio_div := X"8000_0000" / ratio_den;
            ratio_mul := ratio_num * ratio_div; 

            ratio := signed('0' & ratio_mul(30 downto 0));

            if sample_ptr < 7 then
                ratio := ratio + 128;
            elsif sample_ptr > 7 then
                ratio := ratio - 128;
            end if;

            -- Phase accumulator ------------------------------------------------------------------
            phase           := phase + ratio; 
            phase_overflow  := phase(31);
            phase           := phase and X"7FFF_FFFF";

            if g2_sample_diff = 2 and phase_overflow = '0' and sample_ptr > 0 then
                sample_ptr := sample_ptr - 1;
            elsif g2_sample_diff = 1 and phase_overflow = '1' and sample_ptr < C_DEPTH-2 then 
                sample_ptr := sample_ptr + 1;
            end if;

            -- Interpolator -----------------------------------------------------------------------
            rd_ptr_a := wr_ptr + sample_ptr;
            rd_ptr_b := wr_ptr + sample_ptr + 1;

            sample_sub := sample_buf(to_integer(rd_ptr_b)) - sample_buf(to_integer(rd_ptr_a));

            if sample_sub < -1000 then
                sample_sub := sample_sub + 2000;
            elsif sample_sub > 1000 then
                sample_sub := sample_sub - 2000;
            end if;

            sample_mul  := sample_sub * phase;
            sample      := (sample_buf(to_integer(rd_ptr_a))(16 downto 0) & "000" & X"000") +
                            sample_mul(47 downto 16);

            sample_mod  := accum_mod(accum_mul(sample, C_ENCODER_GAIN));

            pos_in <= sample_mod;

            -- Debug ------------------------------------------------------------------------------
            s_sample_ptr        <= sample_ptr;
            s_wr_ptr            <= wr_ptr;
            s_rd_ptr_a          <= rd_ptr_a;
            s_rd_ptr_b          <= rd_ptr_b;

            sample_diff         := sample - sample_d;
            sample_d            := sample;
            sample_mod_diff     := accum_minus(sample_mod, sample_mod_d);
            sample_mod_d        := sample_mod;

            s_ratio_num         <= ratio_num;
            s_ratio_den         <= ratio_den;
            s_ratio_div         <= ratio_div;

            r_sample_vel        <= sample_pos;
--            r_ratio             <= to_real(ratio(31 downto 16));
--            r_phase             <= to_real(phase(31 downto 16));
            r_ratio             <= real(to_integer(ratio)) / 2.0**31;
            r_phase             <= real(to_integer(phase)) / 2.0**31;
            r_sample            <= to_real(sample);
            r_sample_mod        <= to_real(sample_mod);
            r_sample_sub        <= real(to_integer(sample_sub));
            r_sample_diff       <= to_real(sample_diff);
            r_sample_mod_diff   <= to_real(sample_mod_diff);

            for i in 0 to C_DEPTH-1 loop
                r_sample_buf(i) <= real(to_integer(sample_buf(i)));
            end loop;
        end if;
    end process mc_p;

    u_vel_double : entity work.vel_double
        port map (
            clk => clk_mc,
            rst => '0',

            pos_in      => pos_in,
            torque_in   => (others => '0'),

            p_pos_out   => pos_out
        );

end architecture tb;

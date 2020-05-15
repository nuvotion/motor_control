-- ghdl_flags: --stop-time=1us --wave=curpid_tb.ghw

-- ghdl_deps: fixed.vhd
-- ghdl_deps: model_rl.vhd
-- ghdl_deps: curpid.vhd

use std.textio.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

use work.fixed.all;

entity curpid_tb is begin
end entity curpid_tb;

architecture tb of curpid_tb is

    signal clk : std_logic := '1';
    signal rst : std_logic := '1';

    type cmd_rom_t is array (natural range <>) of signed(31 downto 0);
    constant C_I_CMD : cmd_rom_t := (
        to_accum(0.0), to_accum(8.0), to_accum(-10.0));

    signal voltage  : signed(31 downto 0);
    signal current  : signed(31 downto 0);

    signal i_count  : natural; 
    signal i_seq    : natural; 
    signal i_cmd    : signed(31 downto 0);

begin

    clk <= not clk after 0.5 ns;
    rst <= '0' after 4 ns;

    stim_p : process(clk) begin
        if rising_edge(clk) then
            if rst = '0' then
                i_count <= i_count + 1;
                if i_count = 100 then
                    i_count <= 0;
                    i_seq <= i_seq + 1;
                    if i_seq = C_I_CMD'high then
                        i_seq <= 0;
                    end if;
                end if;
            end if;
        end if;
    end process stim_p;

    i_cmd <= C_I_CMD(i_seq);

    u_mot : entity work.model_rl
        port map (
            clk     => clk,
            rst     => '0',

            voltage => voltage,
            current => current
        );

    u_curpid : entity work.curpid
        port map (
            clk     => clk,
            rst     => '0',

            i_cmd   => i_cmd,
--            i_fb    => (others => '0'),
            i_fb    => current,
            u_out   => voltage
        );
end architecture tb;

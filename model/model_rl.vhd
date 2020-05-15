library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.fixed.all;

entity model_rl is
    port (
        clk : in  std_logic;
        rst : in  std_logic;

        voltage     : in  signed(31 downto 0);
        current     : out signed(31 downto 0)
    );
end entity model_rl;

architecture float_iir of model_rl is

    constant C_F_PERIOD : real := 0.0002;
    constant C_R        : real := 1.70;
    constant C_L        : real := 0.0026;

    constant C_B_0  : real := C_F_PERIOD/(C_L + C_R*C_F_PERIOD);
    constant C_A_1  : real := C_L/(C_L + C_R*C_F_PERIOD);

    signal r_voltage    : real := 0.0;
    signal r_current    : real := 0.0;
    signal r_current_d  : real := 0.0;

begin

    r_voltage <= to_real(voltage);

    r_current <= r_voltage      * C_B_0 + 
                 r_current_d    * C_A_1;

    current <= to_accum(r_current);

    mem_p : process(clk) begin
        if rising_edge(clk) then
            r_current_d <= r_current;
        end if;
    end process mem_p;

end architecture float_iir;

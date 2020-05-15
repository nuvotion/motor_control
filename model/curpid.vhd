library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.fixed.all;

entity curpid is
    port (
        clk : in  std_logic;
        rst : in  std_logic;

        i_cmd       : in  signed(31 downto 0);
        i_fb        : in  signed(31 downto 0);
        u_out       : out signed(31 downto 0)
    );
end entity curpid;

architecture fixed of curpid is

    constant C_F_PERIOD         : real := 0.0002;
    constant C_F_R              : real := 1.70;
    constant C_F_L              : real := 0.0026;

    constant C_F_CURPID_X_KP    : real := C_F_L * 1.000 / C_F_PERIOD / 2.0;
    constant C_F_CURPID_X_KI    : real := C_F_R * 1.000 * C_F_PERIOD / C_F_L;

    constant C_PERIOD               : signed(31 downto 0) := to_accum(
                                        C_F_PERIOD);
    constant C_CURPID_X_KP          : signed(31 downto 0) := to_accum(
                                        C_F_CURPID_X_KP);
    constant C_CURPID_X_KP_KI       : signed(31 downto 0) := to_accum(
                                        C_F_CURPID_X_KP * C_F_CURPID_X_KI);
    constant C_CURPID_MAX_CURRENT   : signed(31 downto 0) := to_accum(10.0);
    constant C_CURPID_BUS_3PH       : signed(31 downto 0) := to_accum(77.0);
    constant C_SAT_VOLTAGE          : signed(31 downto 0) := to_accum(
                                        30.0*30.0);
    constant C_SAT_TIME             : signed(31 downto 0) := to_accum(0.0006);

    shared variable v_i_cmd         : signed(31 downto 0) := (others => '0');
    shared variable v_i_error       : signed(31 downto 0) := (others => '0');
    shared variable v_i_error_sum   : signed(31 downto 0) := (others => '0');
    shared variable v_u             : signed(31 downto 0) := (others => '0');
    shared variable v_sat           : unsigned(31 downto 0) := (others => '0');
    shared variable v_error         : std_logic := '0';

    signal r_i_cmd                  : real;
    signal r_i_error                : real;
    signal r_i_error_sum            : real;
    signal r_u                      : real;
    signal r_sat                    : real;

    signal s_i_cmd                  : signed(31 downto 0);
    signal s_i_error                : signed(31 downto 0);
    signal s_i_error_sum            : signed(31 downto 0);
    signal s_u                      : signed(31 downto 0);
    signal s_sat                    : unsigned(31 downto 0);
    signal s_error                  : std_logic;

    function limit(val, lim : signed) return signed is begin
        if val > lim then
            return lim;
        elsif val < -lim then
            return -lim;
        else
            return val;
        end if;
    end function limit;

begin

    vars_p : process(clk) begin
        r_i_cmd         <= to_real(v_i_cmd);
        r_i_error       <= to_real(v_i_error);
        r_i_error_sum   <= to_real(v_i_error_sum);
        r_u             <= to_real(v_u);
        r_sat           <= to_real(signed(v_sat));

        s_i_cmd         <= v_i_cmd;
        s_i_error       <= v_i_error;
        s_i_error_sum   <= v_i_error_sum;
        s_u             <= v_u;
        s_sat           <= v_sat;
        s_error         <= v_error;
    end process vars_p;

    curpid_p : process(clk) begin
        if rising_edge(clk) then
            v_i_cmd         := limit(i_cmd, C_CURPID_MAX_CURRENT);
            v_i_error       := v_i_cmd - i_fb;
            v_u             := accum_mul(v_i_error, C_CURPID_X_KP);
            v_u             := limit(v_u, C_CURPID_BUS_3PH);
            v_i_error_sum   := v_i_error_sum +
                               accum_mul_rnd(v_i_error, C_CURPID_X_KP_KI);

            if v_i_error >= to_accum(0.0) then
                if v_i_error_sum > C_CURPID_BUS_3PH - v_u then
                    v_i_error_sum := C_CURPID_BUS_3PH - v_u;
                end if;
            else
                if v_i_error_sum < -C_CURPID_BUS_3PH - v_u then
                    v_i_error_sum := -C_CURPID_BUS_3PH - v_u;
                end if;
            end if;

            v_u             := v_u + v_i_error_sum;

            if accum_mul(v_u, v_u) > C_SAT_VOLTAGE then
                v_sat := accum_add_sat(signed(v_sat),  C_PERIOD);
            else
                v_sat := accum_add_sat(signed(v_sat), -C_PERIOD);
            end if;

            if v_sat > unsigned(C_SAT_TIME) then
                v_error := '1';
            end if;

            u_out           <= v_u;
        end if;
    end process curpid_p;

end architecture fixed;

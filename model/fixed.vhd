library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

package fixed is

    function to_accum(val : real) return signed;
    function to_real(val : signed) return real;
    function accum_mod(val : signed) return signed;
    function accum_mul(a, b : signed) return signed;
    function accum_mul_rnd(a, b : signed) return signed;
    function accum_minus(a, b : signed) return signed;
    function accum_add_sat(a, b : signed) return unsigned;
    function daccum_mod(val : signed) return signed;
    function daccum_mac(a, b, c : signed) return signed;
    function read_daccum(a : signed) return signed;

    constant C_F_PI     : real := 3.14332;

    constant C_1K   : signed(31 downto 0) := to_accum(1.0);
    constant C_M_PI : signed(31 downto 0) := to_accum(C_F_PI);

end package fixed;

package body fixed is

    function to_accum(val : real) return signed is begin
        return to_signed(integer(val * 2.0**15), 32);
    end function to_accum;

    function to_real(val : signed) return real is begin
        return real(to_integer(val)) / 2.0**15;
    end function to_real;

    function accum_mod(val : signed) return signed is
        variable mod_arg : signed(31 downto 0);
    begin
        mod_arg := val + 103000;
        mod_arg := mod_arg mod 206000;
        if mod_arg(31) = '1' then
            mod_arg := mod_arg + 103000;
        else
            mod_arg := mod_arg - 103000;
        end if;
        return mod_arg;
    end function accum_mod;

    function accum_add_sat(a, b : signed) return unsigned is
        variable ax, bx : signed(33 downto 0);
        variable r      : signed(33 downto 0);
    begin
        ax := a(31) & a(31) & a;
        bx := b(31) & b(31) & b;
        r := ax + bx;
        if r(33) = '1' then
            r := (others => '0');
        elsif r(32) = '1' then
            r := (others => '1');
        end if;
        return unsigned(r(31 downto 0));
    end function accum_add_sat;

    function accum_minus(a, b : signed) return signed is
        variable ax, bx : signed(31 downto 0);
        variable r      : signed(31 downto 0);
    begin
        ax := a;
        bx := b;
        if abs(ax - bx) < C_M_PI then
            r := ax - bx;
        elsif ax > bx then
            r := a - b - (C_M_PI(30 downto 0) & '0');
        else
            r := a - b + (C_M_PI(30 downto 0) & '0');
        end if;
        return r;
    end function accum_minus;

    function accum_mul(a, b : signed) return signed is
        variable ax, bx : signed(31 downto 0);
        variable r      : signed(63 downto 0);
    begin
        ax := a;
        bx := b;
        r := ax * bx;
        return r(46 downto 15);
    end function accum_mul;

    function accum_mul_rnd(a, b : signed) return signed is
        variable ax, bx : signed(31 downto 0);
        variable r      : signed(63 downto 0) := X"0000_0000_0000_4000";
    begin
        ax := a;
        bx := b;
        r := r + (ax * bx);
        return r(46 downto 15);
    end function accum_mul_rnd;

    function daccum_mod(val : signed) return signed is
        constant pi_x1   : signed(63 downto 0) := X"0000_0000_C92C_27A6";
        constant pi_x2   : signed(63 downto 0) := X"0000_0001_9258_4F4C";
        variable mod_arg : signed(63 downto 0);
    begin
        mod_arg := val + pi_x1;
        mod_arg := mod_arg mod pi_x2;
        if mod_arg(63) = '1' then
            mod_arg := mod_arg + pi_x1;
        else
            mod_arg := mod_arg - pi_x1;
        end if;
        return mod_arg;
    end function daccum_mod;

    function daccum_mac(a, b, c : signed) return signed is
        variable ax, bx : signed(31 downto 0);
        variable cx     : signed(63 downto 0);
        variable r      : signed(63 downto 0);
    begin
        ax := a;
        bx := b;
        cx := c;
        r := cx + (ax * bx);
        return r;
    end function daccum_mac;

    function read_daccum(a : signed) return signed is
        variable r      : signed(31 downto 0);
    begin
        r := a(46 downto 15);
        return r;
    end function read_daccum;

end package body fixed;

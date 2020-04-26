static void load_cur_pid(void) {
    load("adc");
    load("curpid");
    load("curpid");
    load("pwm_dc");
}

static void init_cur_pid(void) {
    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("curpid",   1, "rt_prio", 3);
    set_pin_val("pwm_dc",   0, "rt_prio", 6);

    set_pin_val("curpid",   0, "bus_voltage",   BUS_DC);
    set_pin_val("curpid",   0, "sat_voltage",   SAT_VOLTAGE_DC);
    set_pin_val("curpid",   0, "kp",            CURPID_Z_KP);
    set_pin_val("curpid",   0, "kp_ki",         CURPID_Z_KP * CURPID_Z_KI);

    set_pin_val("curpid",   1, "bus_voltage",   BUS_DC);
    set_pin_val("curpid",   1, "sat_voltage",   SAT_VOLTAGE_DC);
    set_pin_val("curpid",   1, "kp",            CURPID_Z_KP);
    set_pin_val("curpid",   1, "kp_ki",         CURPID_Z_KP * CURPID_Z_KI);

    connect_pins("curpid",  0, "id_fb", "adc", 0, "iu_x");
    connect_pins("curpid",  0, "iq_fb", "adc", 0, "iw_x");
    connect_pins("curpid",  1, "id_fb", "adc", 0, "iu_y");
    connect_pins("curpid",  1, "iq_fb", "adc", 0, "iw_y");
  
    connect_pins("pwm_dc", 0, "u_0", "curpid", 0, "ud");
    connect_pins("pwm_dc", 0, "u_1", "curpid", 0, "uq");
    connect_pins("pwm_dc", 0, "u_2", "curpid", 1, "ud");
    connect_pins("pwm_dc", 0, "u_3", "curpid", 1, "uq");
}


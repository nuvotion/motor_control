static void override_en_pins(void) {
    EN0_BYP &= ~(1 << EN0_SHIFT);
    EN1_BYP &= ~(1 << EN1_SHIFT);
}

static void load_cur_pid(void) {
    load("adc");
    load("dq");
    load("dq");
    load("curpid");
    load("curpid");
    load("idq");
    load("idq");
    load("pwm");
}

static void init_cur_pid(void) {
    override_en_pins();

    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("dq",       0, "rt_prio", 2);
    set_pin_val("dq",       1, "rt_prio", 2);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("curpid",   1, "rt_prio", 3);
    set_pin_val("idq",      0, "rt_prio", 4);
    set_pin_val("idq",      1, "rt_prio", 4);
    set_pin_val("pwm",      0, "rt_prio", 6);

    set_pin_val("curpid",   0, "bus_voltage",   BUS_3PH);
    set_pin_val("curpid",   0, "sat_voltage",   SAT_VOLTAGE_3PH);
    set_pin_val("curpid",   0, "kp",            CURPID_X_KP);
    set_pin_val("curpid",   0, "kp_ki",         CURPID_X_KP * CURPID_X_KI);

    set_pin_val("curpid",   1, "bus_voltage",   BUS_3PH);
    set_pin_val("curpid",   1, "sat_voltage",   SAT_VOLTAGE_3PH);
    set_pin_val("curpid",   1, "kp",            CURPID_Y_KP);
    set_pin_val("curpid",   1, "kp_ki",         CURPID_Y_KP * CURPID_Y_KI);

    connect_pins("dq", 0, "u",   "adc", 0, "iu_x");
    connect_pins("dq", 0, "w",   "adc", 0, "iw_x");
    connect_pins("dq", 1, "u",   "adc", 0, "iu_y");
    connect_pins("dq", 1, "w",   "adc", 0, "iw_y");

    connect_pins("curpid", 0, "id_fb",  "dq", 0, "d");
    connect_pins("curpid", 0, "iq_fb",  "dq", 0, "q");
    connect_pins("curpid", 1, "id_fb",  "dq", 1, "d");
    connect_pins("curpid", 1, "iq_fb",  "dq", 1, "q");
  
    connect_pins("idq", 0, "d",   "curpid", 0, "ud");
    connect_pins("idq", 0, "q",   "curpid", 0, "uq");
    connect_pins("idq", 1, "d",   "curpid", 1, "ud");
    connect_pins("idq", 1, "q",   "curpid", 1, "uq");

    connect_pins("pwm", 0, "u_x", "idq", 0, "u");
    connect_pins("pwm", 0, "v_x", "idq", 0, "v");
    connect_pins("pwm", 0, "w_x", "idq", 0, "w");
    connect_pins("pwm", 0, "u_y", "idq", 1, "u");
    connect_pins("pwm", 0, "v_y", "idq", 1, "v");
    connect_pins("pwm", 0, "w_y", "idq", 1, "w");
}


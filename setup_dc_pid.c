static void load_pos_pid(void) {
    load("encoder_dc");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder_dc",   0, "rt_prio", 7);
    set_pin_val("dbg",          0, "rt_prio", 14);

    set_pin_val("encoder_dc",   0, "enable", 1);
    set_pin_val("curpid",       0, "enable", 1);

    connect_pins("dbg", 0, "in0", "pwm_dc",     0, "u_1");
    connect_pins("dbg", 0, "in1", "encoder_dc", 0, "mot_pos_0");
    connect_pins("dbg", 0, "in2", "adc",        0, "iw_x");

    /* Stimulus */
    connect_pins("curpid", 0, "id_cmd", "dbg", 0, "step");
    connect_pins("curpid", 0, "iq_cmd", "dbg", 0, "step");
}

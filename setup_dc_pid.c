static void load_pos_pid(void) {
    //load("encoder");
    load("dbg");
}

static void init_pos_pid(void) {
    //set_pin_val("encoder",   0, "rt_prio", 7);
    set_pin_val("dbg",       0, "rt_prio", 14);

    connect_pins("dbg", 0, "in0", "pwm_dc",     0, "u_1");

    /* Stimulus */
    //connect_pins("curpid", 0, "id_cmd", "dbg", 0, "step");
    connect_pins("pwm_dc", 0, "u_1", "dbg", 0, "step");
}

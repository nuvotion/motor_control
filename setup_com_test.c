static void load_pos_pid(void) {
    load("encoder");
    load("uvw");
    load("fb_switch");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",      0, "rt_prio", 7);
    set_pin_val("uvw",          0, "rt_prio", 8);
    set_pin_val("fb_switch",    0, "rt_prio", 9);
    set_pin_val("dbg",          0, "rt_prio", 14);

    set_pin_val("fb_switch", 0, "polecount",         2);
    set_pin_val("fb_switch", 0, "en",                1);
    set_pin_val("fb_switch", 0, "mot_polecount",     1);
    set_pin_val("fb_switch", 0, "com_polecount",     2);
    set_pin_val("fb_switch", 0, "mot_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_state",         3);

    connect_pins("uvw", 0, "u", "encoder", 0, "u");
    connect_pins("uvw", 0, "v", "encoder", 0, "v");
    connect_pins("uvw", 0, "w", "encoder", 0, "w");

    connect_pins("fb_switch", 0, "com_abs_pos", "uvw",      0, "pos");
    connect_pins("fb_switch", 0, "mot_abs_pos", "encoder",  0, "mot_abs_pos");
    connect_pins("fb_switch", 0, "mot_state",   "encoder",  0, "mot_state");

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "uvw",        0, "pos");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_abs_pos");
    connect_pins("dbg", 0, "in3", "fb_switch",  0, "com_fb");
    connect_pins("dbg", 0, "in4", "fb_switch",  0, "current_com_pos");

    /* Stimulus */
    connect_pins("dq",  0, "pos", "dbg", 0, "angle");
    connect_pins("idq", 0, "pos", "dbg", 0, "angle");
    set_pin_val("curpid", 0, "id_cmd", 0.3);
    set_pin_val("curpid", 0, "iq_cmd", 0);
}

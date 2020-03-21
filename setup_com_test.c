static void load_pos_pid(void) {
    load("encoder");
    load("fb_switch");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",   0, "rt_prio", 7);
    set_pin_val("fb_switch", 0, "rt_prio", 9);
    set_pin_val("dbg",       0, "rt_prio", 14);

    set_pin_val("fb_switch", 0, "polecount",          2);
    set_pin_val("fb_switch", 0, "mot_polecount",      1);
    set_pin_val("fb_switch", 0, "com_polecount",      2);
    set_pin_val("fb_switch", 0, "mot_offset", M_PI/6.0K);
    set_pin_val("fb_switch", 0, "com_offset", M_PI/6.0K);

    connect_pins("fb_switch", 0, "com_abs_pos", "encoder",  0, "com_abs_pos");
    connect_pins("fb_switch", 0, "mot_abs_pos", "encoder",  0, "mot_abs_pos");
    connect_pins("fb_switch", 0, "mot_state",   "encoder",  0, "mot_state");

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "encoder",    0, "com_abs_pos");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_abs_pos");
    connect_pins("dbg", 0, "in3", "fb_switch",  0, "com_fb");
    connect_pins("dbg", 0, "in4", "fb_switch",  0, "current_com_pos");

    /* Stimulus */
    connect_pins("dq",     0, "pos",    "dbg", 0, "angle");
    connect_pins("idq",    0, "pos",    "dbg", 0, "angle");
    connect_pins("curpid", 0, "id_cmd", "dbg", 0, "step");
}

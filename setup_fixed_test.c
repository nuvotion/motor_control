static void load_pos_pid(void) {
    load("dbg");
    load("vel");
}

static void init_pos_pid(void) {
    set_pin_val("vel", 0, "rt_prio", 10);
    set_pin_val("dbg", 0, "rt_prio", 14);

    connect_pins("vel", 0, "pos_in", "dbg",        0, "angle");

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "dbg",        0, "step");
    connect_pins("dbg", 0, "in2", "vel",        0, "vel");
    connect_pins("dbg", 0, "in3", "vel",        0, "pos_out");
}

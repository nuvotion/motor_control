static void load_pos_pid(void) {
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("dbg", 0, "rt_prio", 14);

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "dbg",        0, "step");
}

static void load_pos_pid(void) {
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("dbg", 0, "rt_prio", 14);

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "dbg",        0, "step");
    connect_pins("dbg", 0, "in2", "dq",         0, "d");
    connect_pins("dbg", 0, "in3", "dq",         0, "q");
    connect_pins("dbg", 0, "in4", "curpid",     0, "ud");

    /* Stimulus */
    connect_pins("dq",     0, "pos",    "dbg", 0, "angle");
    connect_pins("idq",    0, "pos",    "dbg", 0, "angle");
    connect_pins("curpid", 0, "id_cmd", "dbg", 0, "step");
}

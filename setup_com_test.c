static void load_pos_pid(void) {
    load("encoder");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",   0, "rt_prio", 7);
    set_pin_val("dbg",       0, "rt_prio", 14);

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "encoder",    0, "com_pos");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_pos");
    connect_pins("dbg", 0, "in3", "dq",         0, "d");
    connect_pins("dbg", 0, "in4", "curpid",     0, "ud");

    /* Stimulus */
    connect_pins("dq",     0, "pos",    "dbg", 0, "angle");
    connect_pins("idq",    0, "pos",    "dbg", 0, "angle");
    connect_pins("curpid", 0, "id_cmd", "dbg", 0, "step");
}

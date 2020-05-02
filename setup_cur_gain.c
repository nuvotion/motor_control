static void load_pos_pid(void) {
    load("encoder");
    load("step_dir");
    load("vel");
    load("vel");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",      0, "rt_prio",  7);
    set_pin_val("step_dir",     0, "rt_prio",  7);
    set_pin_val("vel",          0, "rt_prio", 10);
    set_pin_val("vel",          1, "rt_prio", 10);
    set_pin_val("dbg",          0, "rt_prio", 14);

    /* Need to set up VEL_ACC_LP */
    set_pin_val("curpid",  0, "enable", 1);
    set_pin_val("curpid",  1, "enable", 1);
    set_pin_val("encoder", 0, "enable", 1);
    set_pin_val("dbg",     0, "enable", 1);

    /* Commutation motor model - generates position for dq/idq */
    connect_pins("vel", 2, "pos_in", "encoder", 0, "com_pos_y"); 

    /* Position motor model */
    connect_pins("vel", 1, "torque", "dbg",     0, "step2");
    connect_pins("vel", 1, "pos_in", "vel",     1, "pos_out");

    /* Measure motor velocity */
    connect_pins("vel", 0, "pos_in", "encoder", 0, "mot_pos_y");

    /* Drive curpid loop */
    connect_pins("curpid", 1, "iq_cmd", "dbg",      0, "step");
    connect_pins("dq",     1, "pos",    "encoder",  0, "com_pos_y");
    connect_pins("idq",    1, "pos",    "encoder",  0, "com_pos_y");

    /* Debug */
    connect_pins("dbg", 0, "in0", "dbg",        0, "step2");
    connect_pins("dbg", 0, "in1", "vel",        1, "vel");
    connect_pins("dbg", 0, "in2", "vel",        0, "vel");
    connect_pins("dbg", 0, "in3", "dbg",        0, "step");
    connect_pins("dbg", 0, "in4", "dq",         1, "q");
}

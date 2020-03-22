static void load_pos_pid(void) {
    load("encoder");
    load("step_dir");
    load("vel");
    load("vel");
    load("vel");
    load("pid");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",      0, "rt_prio",  7);
    set_pin_val("step_dir",     0, "rt_prio",  7);
    set_pin_val("vel",          0, "rt_prio", 10);
    set_pin_val("vel",          1, "rt_prio", 10);
    set_pin_val("vel",          2, "rt_prio", 10);
    set_pin_val("pid",          0, "rt_prio", 12);
    set_pin_val("dbg",          0, "rt_prio", 14);

    /* Commutation motor model - generates position for dq/idq */
    connect_pins("vel", 2, "pos_in", "encoder",     0, "com_pos"); 
    connect_pins("vel", 2, "torque", "pid",         0, "torque_cor_cmd");

    /* Position motor model */
    connect_pins("vel", 1, "pos_in", "encoder", 0, "mot_pos");
    connect_pins("vel", 1, "torque", "pid",     0, "torque_cor_cmd");

    /* External command */
    connect_pins("vel", 0, "pos_in", "step_dir", 0, "pos");

    /* Position PID */
    connect_pins("pid", 0, "pos_ext_cmd",   "step_dir", 0, "pos");
    connect_pins("pid", 0, "vel_ext_cmd",   "vel",      0, "vel");
    connect_pins("pid", 0, "pos_fb",        "encoder",  0, "mot_pos");
    connect_pins("pid", 0, "vel_fb",        "vel",      1, "vel");

    /* Drive curpid loop */
    connect_pins("curpid", 0, "iq_cmd", "pid",  0, "cur_cor_cmd");
    connect_pins("dq",     0, "pos",    "vel",  2, "pos_out");
    connect_pins("idq",    0, "pos",    "vel",  2, "pos_out");

    /* Debug */
    connect_pins("dbg", 0, "in0", "step_dir",   0, "pos");
    connect_pins("dbg", 0, "in1", "vel",        0, "vel");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_pos");
    connect_pins("dbg", 0, "in3", "vel",        1, "vel");
    connect_pins("dbg", 0, "in4", "pid",        0, "torque_cor_cmd");
    /*
    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "dbg",        0, "step");
    connect_pins("dbg", 0, "in2", "vel",        1, "vel");
    connect_pins("dbg", 0, "in3", "vel",        1, "pos_out");
    connect_pins("dbg", 0, "in4", "vel",        1, "dbg");
    */
    /*
    connect_pins("dbg", 0, "in0", "pid",        0, "torque_cor_cmd");
    connect_pins("dbg", 0, "in1", "pid",        0, "cur_cor_cmd");
    connect_pins("dbg", 0, "in2", "vel",        0, "vel");         
    connect_pins("dbg", 0, "in3", "vel",        1, "vel");
    connect_pins("dbg", 0, "in4", "dq",         0, "q");
    */
}

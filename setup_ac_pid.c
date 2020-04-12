static void load_pos_pid(void) {
    load("encoder");
    load("step_dir");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("pid");
    load("pid");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",      0, "rt_prio",  7);
    set_pin_val("step_dir",     0, "rt_prio",  7);
    set_pin_val("vel",          0, "rt_prio", 10);
    set_pin_val("vel",          1, "rt_prio", 10);
    set_pin_val("vel",          2, "rt_prio", 10);
    set_pin_val("vel",          3, "rt_prio", 10);
    set_pin_val("vel",          4, "rt_prio", 10);
    set_pin_val("vel",          5, "rt_prio", 10);
    set_pin_val("pid",          0, "rt_prio", 12);
    set_pin_val("pid",          1, "rt_prio", 12);
    set_pin_val("dbg",          0, "rt_prio", 14);

    set_pin_val("pid",  0, "pos_p",     PID_POS_P_X);
    set_pin_val("pid",  0, "vel_p",     PID_VEL_P_X);
    set_pin_val("pid",  0, "vel_i",     PID_VEL_I_PERIOD_X);
    set_pin_val("pid",  0, "cur_gain",  PID_CUR_GAIN_X);

    set_pin_val("pid",  1, "pos_p",     PID_POS_P_Y);
    set_pin_val("pid",  1, "vel_p",     PID_VEL_P_Y);
    set_pin_val("pid",  1, "vel_i",     PID_VEL_I_PERIOD_Y);
    set_pin_val("pid",  1, "cur_gain",  PID_CUR_GAIN_Y);
    
    /* Start up condition */
    connect_pins("curpid",  0, "enable", "step_dir", 0, "enable");
    connect_pins("curpid",  1, "enable", "step_dir", 0, "enable");
    connect_pins("encoder", 0, "enable", "step_dir", 0, "enable");

    /* Commutation motor model - generates position for dq/idq */
    connect_pins("vel", 2, "pos_in", "encoder",     0, "com_pos_x"); 
    connect_pins("vel", 2, "torque", "pid",         0, "torque_cor_cmd");
    connect_pins("vel", 5, "pos_in", "encoder",     0, "com_pos_y"); 
    connect_pins("vel", 5, "torque", "pid",         1, "torque_cor_cmd");

    /* Position motor model */
    connect_pins("vel", 1, "pos_in", "encoder", 0, "mot_pos_x");
    connect_pins("vel", 1, "torque", "pid",     0, "torque_cor_cmd");
    connect_pins("vel", 4, "pos_in", "encoder", 0, "mot_pos_y");
    connect_pins("vel", 4, "torque", "pid",     1, "torque_cor_cmd");

    /* External command */
    connect_pins("vel", 0, "pos_in", "step_dir", 0, "pos_x");
    connect_pins("vel", 3, "pos_in", "step_dir", 0, "pos_y");

    /* Position PID */
    connect_pins("pid", 0, "pos_ext_cmd",   "step_dir", 0, "pos_x");
    connect_pins("pid", 0, "vel_ext_cmd",   "vel",      0, "vel");
    connect_pins("pid", 0, "pos_fb",        "encoder",  0, "mot_pos_x");
    connect_pins("pid", 0, "vel_fb",        "vel",      1, "vel");
    connect_pins("pid", 1, "pos_ext_cmd",   "step_dir", 0, "pos_y");
    connect_pins("pid", 1, "vel_ext_cmd",   "vel",      3, "vel");
    connect_pins("pid", 1, "pos_fb",        "encoder",  0, "mot_pos_y");
    connect_pins("pid", 1, "vel_fb",        "vel",      4, "vel");

    /* Drive curpid loop */
    connect_pins("curpid", 0, "iq_cmd", "pid",  0, "cur_cor_cmd");
    connect_pins("dq",     0, "pos",    "vel",  2, "pos_out");
    connect_pins("idq",    0, "pos",    "vel",  2, "pos_out");
    connect_pins("curpid", 1, "iq_cmd", "pid",  1, "cur_cor_cmd");
    connect_pins("dq",     1, "pos",    "vel",  5, "pos_out");
    connect_pins("idq",    1, "pos",    "vel",  5, "pos_out");

    /* Debug */
#if 1
    connect_pins("dbg", 0, "in0", "step_dir",   0, "pos_y");
    connect_pins("dbg", 0, "in1", "vel",        3, "vel");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_pos_y");
    connect_pins("dbg", 0, "in3", "vel",        4, "vel");
    connect_pins("dbg", 0, "in4", "pid",        1, "torque_cor_cmd");
#endif
#if 0
    connect_pins("dbg", 0, "in0", "step_dir",   0, "pos_x");
    connect_pins("dbg", 0, "in1", "vel",        0, "vel");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_pos_x");
    connect_pins("dbg", 0, "in3", "vel",        1, "vel");
    connect_pins("dbg", 0, "in4", "pid",        0, "torque_cor_cmd");
#endif
#if 0
    connect_pins("dbg", 0, "in0", "curpid",     0, "iq_cmd");
    connect_pins("dbg", 0, "in1", "curpid",     0, "iq_fb");
    connect_pins("dbg", 0, "in2", "curpid",     0, "error");
    connect_pins("dbg", 0, "in3", "curpid",     0, "dbg_sat");
    connect_pins("dbg", 0, "in4", "curpid",     0, "uq");
#endif
}

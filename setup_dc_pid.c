static void load_pos_pid(void) {
    load("encoder_dc");
    load("ufm");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("vel");
    load("pid");
    load("pid");
    load("pid");
    load("pid");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder_dc",   0, "rt_prio",  7);
    set_pin_val("ufm",          0, "rt_prio",  7);
    set_pin_val("vel",          0, "rt_prio", 10);
    set_pin_val("vel",          1, "rt_prio", 10);
    set_pin_val("vel",          2, "rt_prio", 10);
    set_pin_val("vel",          3, "rt_prio", 10);
    set_pin_val("vel",          4, "rt_prio", 10);
    set_pin_val("vel",          5, "rt_prio", 10);
    set_pin_val("vel",          6, "rt_prio", 10);
    set_pin_val("vel",          7, "rt_prio", 10);
    set_pin_val("pid",          0, "rt_prio", 12);
    set_pin_val("pid",          1, "rt_prio", 12);
    set_pin_val("pid",          2, "rt_prio", 12);
    set_pin_val("pid",          3, "rt_prio", 12);
    set_pin_val("dbg",          0, "rt_prio", 14);

    set_pin_val("pid",  0, "pos_p",     PID_POS_P_Z);
    set_pin_val("pid",  0, "vel_p",     PID_VEL_P_Z);
    set_pin_val("pid",  0, "vel_i",     PID_VEL_I_PERIOD_Z);
    set_pin_val("pid",  0, "cur_gain",  PID_CUR_GAIN_Z);

    set_pin_val("pid",  1, "pos_p",     PID_POS_P_Z);
    set_pin_val("pid",  1, "vel_p",     PID_VEL_P_Z);
    set_pin_val("pid",  1, "vel_i",     PID_VEL_I_PERIOD_Z);
    set_pin_val("pid",  1, "cur_gain",  PID_CUR_GAIN_Z);
    
    set_pin_val("pid",  2, "pos_p",     PID_POS_P_Z);
    set_pin_val("pid",  2, "vel_p",     PID_VEL_P_Z);
    set_pin_val("pid",  2, "vel_i",     PID_VEL_I_PERIOD_Z);
    set_pin_val("pid",  2, "cur_gain",  PID_CUR_GAIN_Z);
    
    set_pin_val("pid",  3, "pos_p",     PID_POS_P_Z);
    set_pin_val("pid",  3, "vel_p",     PID_VEL_P_Z);
    set_pin_val("pid",  3, "vel_i",     PID_VEL_I_PERIOD_Z);
    set_pin_val("pid",  3, "cur_gain",  PID_CUR_GAIN_Z);
    
    /* Report error */
    connect_pins("dbg",     0, "error0", "curpid",  0, "error");
    connect_pins("dbg",     0, "error1", "curpid",  1, "error");
    
    /* Start up condition */
    connect_pins("curpid",      0, "enable", "ufm", 0, "enable");
    connect_pins("curpid",      1, "enable", "ufm", 0, "enable");
    connect_pins("encoder_dc",  0, "enable", "ufm", 0, "enable");

    /* Position motor model */
    connect_pins("vel", 1, "pos_in", "encoder_dc",  0, "mot_pos_0");
    connect_pins("vel", 1, "torque", "pid",         0, "torque_cor_cmd");
    connect_pins("vel", 3, "pos_in", "encoder_dc",  0, "mot_pos_1");
    connect_pins("vel", 3, "torque", "pid",         1, "torque_cor_cmd");
    connect_pins("vel", 5, "pos_in", "encoder_dc",  0, "mot_pos_2");
    connect_pins("vel", 5, "torque", "pid",         2, "torque_cor_cmd");
    connect_pins("vel", 7, "pos_in", "encoder_dc",  0, "mot_pos_3");
    connect_pins("vel", 7, "torque", "pid",         3, "torque_cor_cmd");

    /* External command */
    connect_pins("vel", 0, "pos_in", "ufm", 0, "pos_a");
    connect_pins("vel", 2, "pos_in", "ufm", 0, "pos_b");
    connect_pins("vel", 4, "pos_in", "ufm", 0, "pos_c");
    connect_pins("vel", 6, "pos_in", "ufm", 0, "pos_d");

    /* Position PID */
    connect_pins("pid", 0, "pos_ext_cmd",   "ufm",          0, "pos_a");
    connect_pins("pid", 0, "vel_ext_cmd",   "vel",          0, "vel");
    connect_pins("pid", 0, "pos_fb",        "encoder_dc",   0, "mot_pos_0");
    connect_pins("pid", 0, "vel_fb",        "vel",          1, "vel");
    connect_pins("pid", 1, "pos_ext_cmd",   "ufm",          0, "pos_b");
    connect_pins("pid", 1, "vel_ext_cmd",   "vel",          2, "vel");
    connect_pins("pid", 1, "pos_fb",        "encoder_dc",   0, "mot_pos_1");
    connect_pins("pid", 1, "vel_fb",        "vel",          3, "vel");
    connect_pins("pid", 2, "pos_ext_cmd",   "ufm",          0, "pos_c");
    connect_pins("pid", 2, "vel_ext_cmd",   "vel",          4, "vel");
    connect_pins("pid", 2, "pos_fb",        "encoder_dc",   0, "mot_pos_2");
    connect_pins("pid", 2, "vel_fb",        "vel",          5, "vel");
    connect_pins("pid", 3, "pos_ext_cmd",   "ufm",          0, "pos_d");
    connect_pins("pid", 3, "vel_ext_cmd",   "vel",          6, "vel");
    connect_pins("pid", 3, "pos_fb",        "encoder_dc",   0, "mot_pos_3");
    connect_pins("pid", 3, "vel_fb",        "vel",          7, "vel");

    /* Drive curpid loop */
    connect_pins("curpid", 0, "id_cmd", "pid",  0, "cur_cor_cmd");
    connect_pins("curpid", 0, "iq_cmd", "pid",  1, "cur_cor_cmd");
    connect_pins("curpid", 1, "id_cmd", "pid",  2, "cur_cor_cmd");
    connect_pins("curpid", 1, "iq_cmd", "pid",  3, "cur_cor_cmd");

    /* Debug */
#if 1
    connect_pins("dbg", 0, "in0", "ufm",        0, "pos_a");
    connect_pins("dbg", 0, "in1", "vel",        0, "vel");
    connect_pins("dbg", 0, "in2", "encoder_dc", 0, "mot_pos_0");
    connect_pins("dbg", 0, "in3", "vel",        1, "vel");
    connect_pins("dbg", 0, "in4", "pid",        0, "torque_cor_cmd");
#endif
#if 0
    connect_pins("dbg", 0, "in0", "ufm",        0, "pos_b");
    connect_pins("dbg", 0, "in1", "vel",        2, "vel");
    connect_pins("dbg", 0, "in2", "encoder_dc", 0, "mot_pos_1");
    connect_pins("dbg", 0, "in3", "vel",        3, "vel");
    connect_pins("dbg", 0, "in4", "pid",        1, "torque_cor_cmd");
#endif
#if 0
    connect_pins("dbg", 0, "in0", "ufm",        0, "pos_c");
    connect_pins("dbg", 0, "in1", "vel",        4, "vel");
    connect_pins("dbg", 0, "in2", "encoder_dc", 0, "mot_pos_2");
    connect_pins("dbg", 0, "in3", "vel",        5, "vel");
    connect_pins("dbg", 0, "in4", "pid",        2, "torque_cor_cmd");
#endif
#if 0
    connect_pins("dbg", 0, "in0", "ufm",        0, "pos_d");
    connect_pins("dbg", 0, "in1", "vel",        6, "vel");
    connect_pins("dbg", 0, "in2", "encoder_dc", 0, "mot_pos_3");
    connect_pins("dbg", 0, "in3", "vel",        7, "vel");
    connect_pins("dbg", 0, "in4", "pid",        3, "torque_cor_cmd");
#endif
}

static void load_pos_pid(void) {
    load("encoder");
    load("uvw");
    load("fb_switch");
    load("vel");
    load("vel");
    load("vel");
    load("pid");
    //load("pmsm_limits");
    load("pmsm_ttc");
    load("dbg");
}

static void init_pos_pid(void) {
    set_pin_val("encoder",      0, "rt_prio",  7);
    set_pin_val("uvw",          0, "rt_prio",  8);
    set_pin_val("fb_switch",    0, "rt_prio",  9);
    set_pin_val("vel",          0, "rt_prio", 10);
    set_pin_val("vel",          1, "rt_prio", 10);
    set_pin_val("vel",          2, "rt_prio", 10);
    //set_pin_val("pmsm_limits",  0, "rt_prio", 11);
    set_pin_val("pid",          0, "rt_prio", 12);
    set_pin_val("pmsm_ttc",     0, "rt_prio", 13);
    set_pin_val("dbg",          0, "rt_prio", 14);

    set_pin_val("fb_switch", 0, "en",                1);
    set_pin_val("fb_switch", 0, "polecount",         2);
    set_pin_val("fb_switch", 0, "mot_polecount",     1);
    set_pin_val("fb_switch", 0, "com_polecount",     2);
    set_pin_val("fb_switch", 0, "mot_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_state",         3);

    set_pin_val("vel", 0, "en", 1);
    set_pin_val("vel", 1, "en", 1);
    set_pin_val("vel", 2, "en", 1);
    set_pin_val("vel", 0, "w",  200);
    set_pin_val("vel", 1, "w",  200);
    set_pin_val("vel", 2, "w",  200);
    set_pin_val("vel", 1, "j",  CONF_J);
    set_pin_val("vel", 2, "j",  CONF_J);

    /*
    set_pin_val("pmsm_limits", 0, "r",            CONF_R);
    set_pin_val("pmsm_limits", 0, "psi",        CONF_PSI);
    set_pin_val("pmsm_limits", 0, "polecount",         2);
    set_pin_val("pmsm_limits", 0, "ac_volt",     BUS_3PH);
    */

    set_pin_val("pid", 0, "enable",           1);
    set_pin_val("pid", 0, "j",           CONF_J);
    //set_pin_val("pid", 0, "pos_p",          100); // Default
    //set_pin_val("pid", 0, "vel_p",         2000); // Default
    //set_pin_val("pid", 0, "vel_i",            5); // Default
    set_pin_val("pid", 0, "pos_p",           10); // Default
    set_pin_val("pid", 0, "vel_p",          100); // Default
    set_pin_val("pid", 0, "vel_i",            0); // Default
    set_pin_val("pid", 0, "max_usr_vel",    800); // Default
    set_pin_val("pid", 0, "max_usr_acc",  80000); // Default
    set_pin_val("pid", 0, "max_usr_torque", 5.7); // From sm060

    set_pin_val("pmsm_ttc", 0, "psi",  CONF_PSI);
    set_pin_val("pmsm_ttc", 0, "polecount",   2);

    /*
    connect_pins("pid", 0, "max_torque", "pmsm_limits", 0, "max_torque");
    connect_pins("pid", 0, "min_torque", "pmsm_limits", 0, "min_torque");
    connect_pins("pid", 0, "max_vel",    "pmsm_limits", 0, "abs_max_vel");
    */
    /* These are actually static, since we don't measure the bus voltage */
    set_pin_val("pid", 0, "max_torque",  BUS_3PH / CONF_R * CONF_PSI * 3);
    set_pin_val("pid", 0, "min_torque", -BUS_3PH / CONF_R * CONF_PSI * 3);
    set_pin_val("pid", 0, "max_vel",     BUS_3PH / CONF_PSI / 2);

    connect_pins("uvw", 0, "u", "encoder", 0, "u");
    connect_pins("uvw", 0, "v", "encoder", 0, "v");
    connect_pins("uvw", 0, "w", "encoder", 0, "w");

    /* Commutation feedback switch (changes on detection of encoder index) */
    connect_pins("fb_switch", 0, "com_abs_pos", "uvw",      0, "pos");
    connect_pins("fb_switch", 0, "mot_abs_pos", "encoder",  0, "mot_abs_pos");
    connect_pins("fb_switch", 0, "mot_state",   "encoder",  0, "mot_state");

    /* Commutation motor model - generates position for dq/idq */
    connect_pins("vel", 2, "pos_in", "fb_switch",   0, "com_fb"); 
    connect_pins("vel", 2, "torque", "pid",         0, "torque_cor_cmd");

    /* Position motor model, will jump on first index, but should be OK */
    connect_pins("vel", 1, "pos_in", "encoder", 0, "mot_abs_pos");
    connect_pins("vel", 1, "torque", "pid",     0, "torque_cor_cmd");

    /* External command */
    connect_pins("vel", 0, "pos_in", "dbg", 0, "angle");

    /* Position PID */
    connect_pins("pid", 0, "pos_ext_cmd",   "dbg",      0, "angle");
    connect_pins("pid", 0, "vel_ext_cmd",   "vel",      0, "vel");
    connect_pins("pid", 0, "pos_fb",        "encoder",  0, "mot_abs_pos");
    connect_pins("pid", 0, "vel_fb",        "vel",      1, "vel");

    /* PMSM torque to current */
    connect_pins("pmsm_ttc", 0, "torque", "pid", 0, "torque_cor_cmd");

    /* Drive curpid loop */
    connect_pins("curpid", 0, "iq_cmd", "pmsm_ttc", 0, "cur");
    connect_pins("dq",     0, "pos",    "vel",      2, "pos_out");
    connect_pins("idq",    0, "pos",    "vel",      2, "pos_out");

    /* Debug */
    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "vel",        0, "vel");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_abs_pos");
    connect_pins("dbg", 0, "in3", "vel",        1, "vel");
    connect_pins("dbg", 0, "in4", "pid",        0, "torque_cor_cmd");
}

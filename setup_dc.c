static void load_cur_pid(void) {
    load("adc");
    load("curpid");
    load("curpid");
    load("pwm_dc");
}

static void init_cur_pid(void) {
    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("curpid",   1, "rt_prio", 3);
    set_pin_val("pwm_dc",   0, "rt_prio", 6);

    //set_pin_val("curpid",   0, "kp",    CURPID_X_KP);
    //set_pin_val("curpid",   0, "kp_ki", CURPID_X_KP * CURPID_X_KI);

    //set_pin_val("curpid",   1, "kp",    CURPID_Y_KP);
    //set_pin_val("curpid",   1, "kp_ki", CURPID_Y_KP * CURPID_Y_KI);

    //connect_pins("curpid", 0, "id_fb",  "dq", 0, "d");
    //connect_pins("curpid", 0, "iq_fb",  "dq", 0, "q");
    //connect_pins("curpid", 1, "id_fb",  "dq", 1, "d");
    //connect_pins("curpid", 1, "iq_fb",  "dq", 1, "q");
  
    /*
    connect_pins("pwm_dc", 0, "u_0", "curpid", 0, "ud");
    connect_pins("pwm_dc", 0, "u_1", "curpid", 0, "uq");
    connect_pins("pwm_dc", 0, "u_2", "curpid", 1, "ud");
    connect_pins("pwm_dc", 0, "u_3", "curpid", 1, "uq");
    */
}


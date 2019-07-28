#include <project.h>
#include <math.h>

#include <hal.h>

#include "print.h"

static int usb_up;
uint64_t systime;
volatile int rt_deadline_err;

extern void load(char *); // found in ../../stmbl/shared/hal.c line: 517

static void usb_poll(void) {
    if (USBFS_GetConfiguration()) {
        if (!usb_up) USBFS_CDC_Init();
        usb_up = 1;
    } else {
        usb_up = 0;
    }
}

void print(char *string) {
    if (!usb_up) return;
    while (!USBFS_CDCIsReady()) {}
    USBFS_PutData((uint8_t *)string, strlen(string));
}

uint32_t hal_get_systick_freq() {
  return 0;
}

CY_ISR(rt_irq_handler) {
    hal_run_rt();

    /* RT_TIMER is free running. If the interrupt bit is set at the end of the
     * ISR then the ISR routine has missed the realtime deadline. */
    if (*RT_IRQ_INTC_SET_PD & RT_IRQ__INTC_MASK) {
        RT_RESET_Write(1);
        rt_deadline_err++;
        RT_IRQ_ClearPending();
    }
}

static void init_io(void) {
    EN0_Write(0);
    EN1_Write(0);
    RELAY0_Write(0);
    RELAY1_Write(0);
}

static void enable_drive(uint8_t drive) {
    if (drive == 0) {
        RELAY0_Write(1);
        EN0_Write(1);
    }
    if (drive == 1) {
        RELAY1_Write(1);
        EN1_Write(1);
    }
}

static void set_pin_val(NAME comp, uint32_t inst, NAME pin, float val) {
    volatile hal_pin_inst_t *sink;
    sink = pin_inst_by_name(comp, inst, pin);
    sink->value  = val;
    sink->source = sink;
}

static void connect_pins(
        NAME sink_comp,   uint32_t sink_inst,   NAME sink_pin,
        NAME source_comp, uint32_t source_inst, NAME source_pin) {
    volatile hal_pin_inst_t *source;
    volatile hal_pin_inst_t *sink;
    source = pin_inst_by_name(source_comp, source_inst, source_pin);
    sink   = pin_inst_by_name(sink_comp,   sink_inst,   sink_pin);
    sink->source = source;
}

#define CONF_R      1.6         // Resistance (ohms) from sm060
#define CONF_L      0.008       // Inductance (henry) from sm060
#define CONF_PSI    0.055       // Default electrical torque constant [V*s/rad]
#define CONF_J      0.00007     // Inertia from sm060 [kg*m^2]
//#define BUS_DC      100
#define BUS_DC      15
#define BUS_3PH     (BUS_DC / M_SQRT3 * 0.95)
#define MAX_CURRENT 1

static void load_cur_pid(void) {
    load("adc");
    load("dq");
    load("curpid");
    load("idq");
    load("svm");
    load("pwm");
}

static void init_cur_pid(void) {
    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("dq",       0, "rt_prio", 2);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("idq",      0, "rt_prio", 4);
    set_pin_val("svm",      0, "rt_prio", 5);
    set_pin_val("pwm",      0, "rt_prio", 6);

    set_pin_val("dq",  0, "mode", 2); // PHASE_120_3PH
    set_pin_val("idq", 0, "mode", 2); // PHASE_120_3PH

    set_pin_val("curpid", 0, "rd",            CONF_R);
    set_pin_val("curpid", 0, "rq",            CONF_R);
    set_pin_val("curpid", 0, "ld",            CONF_L);
    set_pin_val("curpid", 0, "lq",            CONF_L);
    set_pin_val("curpid", 0, "psi",         CONF_PSI);
    set_pin_val("curpid", 0, "kp",               0.2); // Default fudge factor
    set_pin_val("curpid", 0, "ki",            0.0004); // From sm060
    set_pin_val("curpid", 0, "max_cur",  MAX_CURRENT); // Current limit (A)
    set_pin_val("curpid", 0, "pwm_volt",     BUS_3PH); // Voltage limit (V)
    set_pin_val("curpid", 0, "en",                 1);
    set_pin_val("curpid", 0, "cmd_mode",           1);

    set_pin_val("pwm", 0, "udc", BUS_DC);

    connect_pins("dq", 0, "pos", "vel", 2, "pos_out");
    connect_pins("dq", 0, "u",   "adc", 0, "iu");
    connect_pins("dq", 0, "w",   "adc", 0, "iw");

    connect_pins("curpid", 0, "iq_cmd", "pmsm_ttc", 0, "cur");
    connect_pins("curpid", 0, "id_fb",  "dq",       0, "d");
    connect_pins("curpid", 0, "iq_fb",  "dq",       0, "q");
  
    connect_pins("idq", 0, "pos", "vel",    2, "pos_out");
    connect_pins("idq", 0, "d",   "curpid", 0, "ud");
    connect_pins("idq", 0, "q",   "curpid", 0, "uq");

    connect_pins("svm", 0, "u", "idq", 0, "u");
    connect_pins("svm", 0, "v", "idq", 0, "v");
    connect_pins("svm", 0, "w", "idq", 0, "w");

    connect_pins("pwm", 0, "u", "svm", 0, "su");
    connect_pins("pwm", 0, "v", "svm", 0, "sv");
    connect_pins("pwm", 0, "w", "svm", 0, "sw");
}

#ifdef COM_TEST
static void load_com_test(void) {
    load("encoder");
    load("uvw");
    load("fb_switch");
    load("dbg");
}

static void init_com_test(void) {
    set_pin_val("encoder",      0, "rt_prio", 7);
    set_pin_val("uvw",          0, "rt_prio", 8);
    set_pin_val("fb_switch",    0, "rt_prio", 9);

    set_pin_val("fb_switch", 0, "polecount",         2);
    set_pin_val("fb_switch", 0, "en",                1);
    set_pin_val("fb_switch", 0, "mot_polecount",     1);
    set_pin_val("fb_switch", 0, "com_polecount",     2);
    set_pin_val("fb_switch", 0, "mot_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_offset", M_PI/6.0);
    set_pin_val("fb_switch", 0, "com_state",         3);

    connect_pins("uvw", 0, "u", "encoder", 0, "u");
    connect_pins("uvw", 0, "v", "encoder", 0, "v");
    connect_pins("uvw", 0, "w", "encoder", 0, "w");

    connect_pins("fb_switch", 0, "com_abs_pos", "uvw",      0, "pos");
    connect_pins("fb_switch", 0, "mot_abs_pos", "encoder",  0, "mot_abs_pos");
    connect_pins("fb_switch", 0, "mot_state",   "encoder",  0, "mot_state");

    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "uvw",        0, "pos");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_abs_pos");
    connect_pins("dbg", 0, "in3", "fb_switch",  0, "com_fb");
    connect_pins("dbg", 0, "in4", "fb_switch",  0, "current_com_pos");

    /* Stimulus */
    connect_pins("dq",  0, "pos", "dbg", 0, "angle");
    connect_pins("idq", 0, "pos", "dbg", 0, "angle");
    set_pin_val("curpid", 0, "id_cmd", 0.3);
    set_pin_val("curpid", 0, "iq_cmd", 0);
}
#else
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
    set_pin_val("pid", 0, "pos_p",          100); // Default
    set_pin_val("pid", 0, "vel_p",         2000); // Default
    set_pin_val("pid", 0, "vel_i",            5); // Default
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

    /* Debug */
    set_pin_val("vel", 0, "w",  10);
    set_pin_val("vel", 1, "w",  100);
    set_pin_val("vel", 2, "w",  100);
    connect_pins("dbg", 0, "in0", "dbg",        0, "angle");
    connect_pins("dbg", 0, "in1", "vel",        0, "vel");
    connect_pins("dbg", 0, "in2", "encoder",    0, "mot_abs_pos");
    connect_pins("dbg", 0, "in3", "pmsm_ttc",   0, "cur");
}
#endif

static void init_hal(void) {
    hal_set_debug_level(2);
    hal_init(0.001, 0.001);

#ifdef COM_TEST
    load_cur_pid();
    load_com_test();
    init_cur_pid();
    init_com_test();
#else
    load_cur_pid();
    load_pos_pid();
    init_cur_pid();
    init_pos_pid();
#endif

    hal_start();
}

int main(void) {
    int i = 0;
    int toggle = 0;

    init_io();

    USBFS_Start(0, USBFS_DWR_VDDD_OPERATION);
    RT_IRQ_StartEx(rt_irq_handler);

    init_hal();

    CyGlobalIntEnable;
    RT_TIMER_Start();
    enable_drive(0);

    for(;;) {
        //hal_run_frt();
        //hal_run_nrt();

        if (i == 10000) {
            hal_run_nrt();
            i = 0;
            LED_1_Write(toggle);
            toggle = !toggle;
        }

        if (rt_deadline_err) LED_0_Write(1);

        usb_poll();
        i++;
    }
}

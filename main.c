#include <project.h>

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

/* Todo - move most of this into hal components */
static void init_io(void) {
    EN0_Write(1);
    EN1_Write(0);
    LED_0_Write(0);
    LED_1_Write(0);
    RELAY0_Write(1);
    RELAY1_Write(0);

    QuadDec_0_Start();
    QuadDec_2_Start();
}

/* Move this into a hal component */
#define PWM_VAL 128
static void motor_step(void) {
    static int pos = 0;
    switch (pos % 3) {
        case 0:
            PWM_0_WriteCompare(PWM_VAL);
            PWM_1_WriteCompare(0);
            PWM_2_WriteCompare(0);
            break;
        case 1:
            PWM_0_WriteCompare(0);
            PWM_1_WriteCompare(PWM_VAL);
            PWM_2_WriteCompare(0);
            break;
        default:
            PWM_0_WriteCompare(0);
            PWM_1_WriteCompare(0);
            PWM_2_WriteCompare(PWM_VAL);
            break;
    }
    pos++;
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

static void init_hal(void) {
    hal_set_debug_level(2);
    hal_init(0.001, 0.001);

    //load_comp("pmsm_limits");
    //load_comp("pmsm_ttc");
    //load_comp("fb_switch");
    load("dbg");
    load("adc");
    load("dq");
    load("curpid");
    load("idq");
    load("svm");
    load("pwm");

    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("dq",       0, "rt_prio", 2);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("idq",      0, "rt_prio", 4);
    set_pin_val("svm",      0, "rt_prio", 5);
    set_pin_val("pwm",      0, "rt_prio", 6);

    //set_pin_val("dq", 0, "pos",  0);
    set_pin_val("dq", 0, "mode", 2); // PHASE_120_3PH

    set_pin_val("curpid", 0, "rd",        1.6); // Resistance (ohms) from sm060
    set_pin_val("curpid", 0, "rq",        1.6);
    set_pin_val("curpid", 0, "ld",      0.008); // Inductance (henry) from sm060
    set_pin_val("curpid", 0, "lq",      0.008);
    set_pin_val("curpid", 0, "psi",     0.055); // default
    set_pin_val("curpid", 0, "kp",        0.2); // default fudge factor
    set_pin_val("curpid", 0, "ki",     0.0004); // default fudge factor
    set_pin_val("curpid", 0, "max_cur",     1); // Current limit (A)
    set_pin_val("curpid", 0, "pwm_volt",  100); // Voltage limit (V)
    set_pin_val("curpid", 0, "en",          1);
    set_pin_val("curpid", 0, "cmd_mode",    1);

    //set_pin_val("idq", 0, "pos",  0);
    set_pin_val("idq", 0, "mode", 2); // PHASE_120_3PH

    set_pin_val("pwm", 0, "udc",  100); // Voltage limit (V)

    connect_pins("dq", 0, "u", "adc", 0, "iu");
    connect_pins("dq", 0, "w", "adc", 0, "iw");

    set_pin_val("curpid", 0, "id_cmd", 0.3); // from pmsm
    set_pin_val("curpid", 0, "iq_cmd", 0);
    connect_pins("curpid", 0, "id_fb", "dq", 0, "d");
    connect_pins("curpid", 0, "iq_fb", "dq", 0, "q");
  
    connect_pins("idq", 0, "d", "curpid", 0, "ud");
    connect_pins("idq", 0, "q", "curpid", 0, "uq");

    connect_pins("svm", 0, "u", "idq", 0, "u");
    connect_pins("svm", 0, "v", "idq", 0, "v");
    connect_pins("svm", 0, "w", "idq", 0, "w");

    connect_pins("pwm", 0, "u", "svm", 0, "su");
    connect_pins("pwm", 0, "v", "svm", 0, "sv");
    connect_pins("pwm", 0, "w", "svm", 0, "sw");

    connect_pins("dbg", 0, "in0", "adc", 0, "iu");
    connect_pins("dbg", 0, "in1", "adc", 0, "iw");
    connect_pins("dbg", 0, "in2", "dq",  0, "d");
    connect_pins("dbg", 0, "in3", "dq",  0, "q");
    connect_pins("dbg", 0, "in4", "pwm", 0, "ou");
    connect_pins("dbg", 0, "in5", "pwm", 0, "ov");
    connect_pins("dbg", 0, "in6", "pwm", 0, "ow");

    /* debug stim */
    connect_pins("dq",  0, "pos", "dbg", 0, "angle");
    connect_pins("idq", 0, "pos", "dbg", 0, "angle");

    hal_start();
}

int main(void) {
    int i = 0;
    int toggle = 0;

    init_io();

    USBFS_Start(0, USBFS_DWR_VDDD_OPERATION);
    RT_IRQ_StartEx(rt_irq_handler);

    init_hal();
    //LED_0_Write(1);

    CyGlobalIntEnable;
    RT_TIMER_Start();

    for(;;) {
        //hal_run_frt();
        //hal_run_nrt();

        if (i == 100000) {
            hal_run_nrt();
            i = 0;
            LED_1_Write(toggle);
            toggle = !toggle;
            //motor_step();
        }

        if (rt_deadline_err) LED_0_Write(1);

        usb_poll();
        i++;
    }
}

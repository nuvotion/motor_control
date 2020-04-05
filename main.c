#include <project.h>

#include <hal.h>
#include <defines.h>

#include "constants.h"

#include "print.h"

static int usb_up;
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
    //EN0_Write(1);
    hal_run_rt();
    //EN0_Write(0);

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
        EN1_Write(1);
    }
    if (drive == 1) {
        RELAY1_Write(1);
        EN0_Write(1);
        EN1_Write(1);
    }
}

static void set_pin_val(NAME comp, uint32_t inst, NAME pin, accum val) {
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

static void load_cur_pid(void) {
    load("adc");
    load("dq");
    load("dq");
    load("curpid");
    load("curpid");
    load("idq");
    load("idq");
    load("pwm");
}

static void init_cur_pid(void) {
    set_pin_val("adc",      0, "rt_prio", 1);
    set_pin_val("dq",       0, "rt_prio", 2);
    set_pin_val("dq",       1, "rt_prio", 2);
    set_pin_val("curpid",   0, "rt_prio", 3);
    set_pin_val("curpid",   1, "rt_prio", 3);
    set_pin_val("idq",      0, "rt_prio", 4);
    set_pin_val("idq",      1, "rt_prio", 4);
    set_pin_val("pwm",      0, "rt_prio", 6);

    set_pin_val("curpid",   0, "kp",    CURPID_X_KP);
    set_pin_val("curpid",   0, "kp_ki", CURPID_X_KP * CURPID_X_KI);

    set_pin_val("curpid",   1, "kp",    CURPID_Y_KP);
    set_pin_val("curpid",   1, "kp_ki", CURPID_Y_KP * CURPID_Y_KI);

    connect_pins("dq", 0, "u",   "adc", 0, "iu_x");
    connect_pins("dq", 0, "w",   "adc", 0, "iw_x");
    connect_pins("dq", 1, "u",   "adc", 0, "iu_y");
    connect_pins("dq", 1, "w",   "adc", 0, "iw_y");

    connect_pins("curpid", 0, "id_fb",  "dq", 0, "d");
    connect_pins("curpid", 0, "iq_fb",  "dq", 0, "q");
    connect_pins("curpid", 1, "id_fb",  "dq", 1, "d");
    connect_pins("curpid", 1, "iq_fb",  "dq", 1, "q");
  
    connect_pins("idq", 0, "d",   "curpid", 0, "ud");
    connect_pins("idq", 0, "d",   "curpid", 0, "ud");
    connect_pins("idq", 1, "q",   "curpid", 1, "uq");
    connect_pins("idq", 1, "q",   "curpid", 1, "uq");

    connect_pins("pwm", 0, "u_x", "idq", 0, "u");
    connect_pins("pwm", 0, "v_x", "idq", 0, "v");
    connect_pins("pwm", 0, "w_x", "idq", 0, "w");
    connect_pins("pwm", 0, "u_y", "idq", 1, "u");
    connect_pins("pwm", 0, "v_y", "idq", 1, "v");
    connect_pins("pwm", 0, "w_y", "idq", 1, "w");
}

#if defined(COM_TEST)
    #include "setup_com_test.c"
#elif defined(CURPID_TEST)
    #include "setup_curpid_test.c"
#elif defined(FIXED_POINT_TEST)
    #include "setup_fixed_test.c"
#else
    #include "setup_normal.c"
#endif

static void init_hal(void) {
    hal_set_debug_level(2);
    hal_init(PERIOD, PERIOD);

    load_cur_pid();
    load_pos_pid();
    init_cur_pid();
    init_pos_pid();

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
    RT_TIMER_WritePeriod(49);
    enable_drive(0);
    enable_drive(1);

    for(;;) {
        //hal_run_frt();
        //hal_run_nrt();

        if (i == 10000) {
            hal_run_nrt();
            i = 0;
            LED_1_Write(toggle);
            toggle = !toggle;

            if (rt_deadline_err) {
                //rt_deadline_err--;
                LED_0_Write(1);
            } else {
                LED_0_Write(0);
            }
        }

        usb_poll();
        i++;
    }
}

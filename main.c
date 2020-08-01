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
    if (CY_INT_SET_PEND_REG & (1 << CY_INT_SYSTICK_IRQN)) {
        rt_deadline_err++;
        CyIntClearPending(CY_INT_SYSTICK_IRQN);
    }
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

#if defined(COM_TEST)
    #include "setup_com_test.c"
#elif defined(CURPID_TEST)
    #include "setup_curpid_test.c"
#elif defined(FIXED_POINT_TEST)
    #include "setup_fixed_test.c"
#elif defined(DC_SERVO)
    #include "setup_dc.c"
    #include "setup_dc_pid.c"
#else
    #include "setup_ac.c"
    #include "setup_ac_pid.c"
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

    USBFS_Start(0, USBFS_DWR_VDDD_OPERATION);

    init_hal();
    enable_drive(0);
    enable_drive(1);
    LED_0_Write(1);

    CySysTickStart();
    CySysTickSetReload(BCLK__BUS_CLK__HZ/5000);
    CySysTickClear();
    CySysTickSetCallback(0, rt_irq_handler);

    CyGlobalIntEnable;

    for(;;) {
        if (i == 10000) {
            hal_run_nrt();
            i = 0;
        }

        if (rt_deadline_err) LED_0_Write(0);

        usb_poll();
        i++;
    }
}

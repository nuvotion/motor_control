#include <project.h>

#include <hal.h>

#include "print.h"

static int usb_up;

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

uint32_t hal_get_systick_value() {
  return 0;
}

uint32_t hal_get_systick_reload() {
  return 0;
}

uint32_t hal_get_systick_freq() {
  return 0;
}

volatile uint64_t systime = 0;

CY_ISR(rt_irq_handler) {
    hal_run_rt();

    systime++;

    RT_TIMER_Stop();
    RT_RESET_Write(1);
    RT_TIMER_Enable();
}

int main(void) {
    int i = 0;

    LED_0_Write(1);

    USBFS_Start(0, USBFS_DWR_VDDD_OPERATION);
    RT_IRQ_StartEx(rt_irq_handler);
    RT_TIMER_Start();
    QuadDec_0_Start();
    CyGlobalIntEnable;

    hal_init(0.0002, 0.00005);

    hal_parse("load test");
    hal_parse("load testrt");

    hal_parse("testrt0.rt_prio = 1.0");

    hal_parse("test0.in = testrt0.out");

    //hal_parse("load pmsm_limits");
    //hal_parse("load pmsm_ttc");
    //hal_parse("load fb_switch");

    //hal_parse("load dq");
    //hal_parse("load idq");
    //hal_parse("load svm");
    //hal_parse("load curpid");

    hal_start();

    for(;;) {
        //hal_run_frt();
        //hal_run_nrt();

        if (i == 100000) {
            //hal_run_nrt();
            print("quad_dec: ");
            print(print_num(QuadDec_0_GetCounter()));
            print("\r\n");
            i = 0;
        }

        usb_poll();
        i++;
    }
}

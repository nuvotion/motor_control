#include <project.h>

#include <hal.h>

#include "print.h"
#include "adc.h"

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

static void init_io(void) {
    EN0_Write(1);
    EN1_Write(0);
    LED_0_Write(1);
    LED_1_Write(1);
    RELAY0_Write(1);
    RELAY1_Write(0);
    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();

    PWM_0_WriteCompare(0);
    PWM_1_WriteCompare(0);
    PWM_2_WriteCompare(0);
    PWM_3_WriteCompare(0);
    PWM_4_WriteCompare1(0);
    PWM_4_WriteCompare2(0);

    QuadDec_0_Start();
    //QuadDec_2_Start();
    ADC_Start();
}

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
    //pos++;
}

static void init_hal(void) {
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
}

int main(void) {
    int i = 0;
    //int uvw;
    int toggle = 0;

    init_io();

    USBFS_Start(0, USBFS_DWR_VDDD_OPERATION);
    RT_IRQ_StartEx(rt_irq_handler);
    RT_TIMER_Start();

    init_hal();

    CyGlobalIntEnable;

    for(;;) {
        //hal_run_frt();
        //hal_run_nrt();

        if (i == 1000000) {
            //hal_run_nrt();
            /*
            print("uvw: ");
            uvw =   (QUAD1_U_Read() << 2) + 
                    (QUAD1_V_Read() << 1) + 
                    (QUAD1_W_Read() << 0);
            print(print_num(uvw));
            print(", quad_dec: ");
            print(print_num(QuadDec_0_GetCounter()));
            print("\r\n");
            */
            print("ch0: "); print(itoa(ADC_GetResult16(0) + 215, 10));
            print(", ch1: "); print(itoa(ADC_GetResult16(1) + 110, 10));
            print(", ch2: "); print(itoa(ADC_GetResult16(2) + 325, 10));
            print(", ch3: "); print(itoa(ADC_GetResult16(3) + 245, 10));
            print("\r\n");
            ADC_StartConvert();
            motor_step();
            LED_0_Write(toggle);
            toggle = !toggle;
            i = 0;
        }

        usb_poll();
        i++;
    }
}

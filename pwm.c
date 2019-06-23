#include <project.h>
#include "hal.h"
#include "defines.h"

HAL_COMP(pwm);

// U V W input in volts
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

// Supply voltage to scale PWM
HAL_PIN(udc);

// Debug
HAL_PIN(ou);
HAL_PIN(ov);
HAL_PIN(ow);

/*
struct pwm_ctx_t {
};
*/

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct pwm_ctx_t *ctx = (struct pwm_ctx_t *) ctx_ptr;
    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();

    PWM_0_WriteCompare(0); // FIXME: Just set this in the configuration
    PWM_1_WriteCompare(0);
    PWM_2_WriteCompare(0);
    PWM_3_WriteCompare(0);
    PWM_4_WriteCompare1(0);
    PWM_4_WriteCompare2(0);
}

static void rt_func(float period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    uint8_t u, v, w;
    //struct pwm_ctx_t *ctx      = (struct pwm_ctx_t *) ctx_ptr;
    struct pwm_pin_ctx_t *pins = (struct pwm_pin_ctx_t *) pin_ptr;
  
    float udc = MAX(PIN(udc), 0.1);

    //convert voltages to PWM output compare values
    u = (uint8_t) (CLAMP(PIN(u), 0.0, udc) / udc * 255);
    v = (uint8_t) (CLAMP(PIN(v), 0.0, udc) / udc * 255);
    w = (uint8_t) (CLAMP(PIN(w), 0.0, udc) / udc * 255);

    PWM_0_WriteCompare(u);
    PWM_1_WriteCompare(v);
    PWM_2_WriteCompare(w);

    PIN(ou) = u;
    PIN(ov) = v;
    PIN(ow) = w;
}

hal_comp_t pwm_comp_struct = {
    .name      = "pwm",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0, //sizeof(struct pwm_ctx_t),
    .pin_count = sizeof(struct pwm_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

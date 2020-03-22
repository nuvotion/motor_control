#include <project.h>
#include "hal.h"
#include "defines.h"
#include "constants.h"

HAL_COMP(pwm);

// U V W input in volts
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

struct pwm_ctx_t {
    int init_samples;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct pwm_ctx_t *ctx = (struct pwm_ctx_t *) ctx_ptr;

    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();

    ctx->init_samples = 1000;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    uint8_t u_pwm, v_pwm, w_pwm;
    struct pwm_ctx_t *ctx      = (struct pwm_ctx_t *) ctx_ptr;
    struct pwm_pin_ctx_t *pins = (struct pwm_pin_ctx_t *) pin_ptr;
  
    accum u = PIN(u);
    accum v = PIN(v);
    accum w = PIN(w);

    accum offset = MIN3(u, v, w); // SVM

    u -= offset;
    v -= offset;
    w -= offset;

    u = CLAMP(u, 0.0K, BUS_DC);
    v = CLAMP(v, 0.0K, BUS_DC);
    w = CLAMP(w, 0.0K, BUS_DC);

    //convert voltages to PWM output compare values
    u_pwm = (uint8_t) (u / BUS_DC * 255K);
    v_pwm = (uint8_t) (v / BUS_DC * 255K);
    w_pwm = (uint8_t) (w / BUS_DC * 255K);

    if (ctx->init_samples) {
        ctx->init_samples--;
    } else {
        PWM_0_WriteCompare(u_pwm);
        PWM_1_WriteCompare(v_pwm);
        PWM_2_WriteCompare(w_pwm);
    }
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
    .ctx_size  = sizeof(struct pwm_ctx_t),
    .pin_count = sizeof(struct pwm_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

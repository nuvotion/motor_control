#include <project.h>
#include "hal.h"
#include "defines.h"
#include "constants.h"

HAL_COMP(pwm);

// U V W input in volts
HAL_PIN(u_x);
HAL_PIN(v_x);
HAL_PIN(w_x);
HAL_PIN(u_y);
HAL_PIN(v_y);
HAL_PIN(w_y);

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
    uint8_t u_pwm_x, v_pwm_x, w_pwm_x;
    uint8_t u_pwm_y, v_pwm_y, w_pwm_y;
    struct pwm_ctx_t *ctx      = (struct pwm_ctx_t *) ctx_ptr;
    struct pwm_pin_ctx_t *pins = (struct pwm_pin_ctx_t *) pin_ptr;
  
    accum u_x = PIN(u_x);
    accum v_x = PIN(v_x);
    accum w_x = PIN(w_x);
    accum u_y = PIN(u_y);
    accum v_y = PIN(v_y);
    accum w_y = PIN(w_y);

    accum offset_x = MIN3(u_x, v_x, w_x); // SVM
    accum offset_y = MIN3(u_y, v_y, w_y); // SVM

    u_x -= offset_x;
    v_x -= offset_x;
    w_x -= offset_x;
    u_y -= offset_y;
    v_y -= offset_y;
    w_y -= offset_y;

    u_x = CLAMP(u_x, 0.0K, BUS_DC);
    v_x = CLAMP(v_x, 0.0K, BUS_DC);
    w_x = CLAMP(w_x, 0.0K, BUS_DC);
    u_y = CLAMP(u_y, 0.0K, BUS_DC);
    v_y = CLAMP(v_y, 0.0K, BUS_DC);
    w_y = CLAMP(w_y, 0.0K, BUS_DC);

    //convert voltages to PWM output compare values
    u_pwm_x = (uint8_t) (u_x * (255K / BUS_DC));
    v_pwm_x = (uint8_t) (v_x * (255K / BUS_DC));
    w_pwm_x = (uint8_t) (w_x * (255K / BUS_DC));
    u_pwm_y = (uint8_t) (u_y * (255K / BUS_DC));
    v_pwm_y = (uint8_t) (v_y * (255K / BUS_DC));
    w_pwm_y = (uint8_t) (w_y * (255K / BUS_DC));

    if (ctx->init_samples) {
        ctx->init_samples--;
    } else {
        PWM_0_WriteCompare(u_pwm_x);
        PWM_1_WriteCompare(v_pwm_x);
        PWM_2_WriteCompare(w_pwm_x);
        PWM_3_WriteCompare(u_pwm_y);
        PWM_4_WriteCompare1(v_pwm_y);
        PWM_4_WriteCompare2(w_pwm_y);
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

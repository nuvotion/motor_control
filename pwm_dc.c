#include <project.h>
#include "hal.h"
#include "defines.h"
#include "constants.h"

HAL_COMP(pwm_dc);

// U V W input in volts
HAL_PIN(u_0);
HAL_PIN(u_1);
HAL_PIN(u_2);
HAL_PIN(u_3);

struct pwm_dc_ctx_t {
    int init_samples;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct pwm_dc_ctx_t *ctx = (struct pwm_dc_ctx_t *) ctx_ptr;

    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();

    ctx->init_samples = 1000;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    uint8_t u_pwm_0, u_pwm_1, u_pwm_2, u_pwm_3;
    uint8_t u_pwm_4, u_pwm_5, u_pwm_6, u_pwm_7;
    struct pwm_dc_ctx_t *ctx      = (struct pwm_dc_ctx_t *) ctx_ptr;
    struct pwm_dc_pin_ctx_t *pins = (struct pwm_dc_pin_ctx_t *) pin_ptr;
  
    accum u_a_0, u_b_0, u_a_1, u_b_1, u_a_2, u_b_2, u_a_3, u_b_3;

    u_a_0 = CLAMP( PIN(u_0), 0.0K, BUS_DC);
    u_b_0 = CLAMP(-PIN(u_0), 0.0K, BUS_DC);
    u_a_1 = CLAMP( PIN(u_1), 0.0K, BUS_DC);
    u_b_1 = CLAMP(-PIN(u_1), 0.0K, BUS_DC);
    u_a_2 = CLAMP( PIN(u_2), 0.0K, BUS_DC);
    u_b_2 = CLAMP(-PIN(u_2), 0.0K, BUS_DC);
    u_a_3 = CLAMP( PIN(u_3), 0.0K, BUS_DC);
    u_b_3 = CLAMP(-PIN(u_3), 0.0K, BUS_DC);

    //convert voltages to PWM output compare values
    u_pwm_0 = (uint8_t) (u_a_0 * (255K / BUS_DC));
    u_pwm_1 = (uint8_t) (u_b_0 * (255K / BUS_DC));
    u_pwm_2 = (uint8_t) (u_a_1 * (255K / BUS_DC));
    u_pwm_3 = (uint8_t) (u_b_1 * (255K / BUS_DC));
    u_pwm_4 = (uint8_t) (u_a_2 * (255K / BUS_DC));
    u_pwm_5 = (uint8_t) (u_b_2 * (255K / BUS_DC));
    u_pwm_6 = (uint8_t) (u_a_3 * (255K / BUS_DC));
    u_pwm_7 = (uint8_t) (u_b_3 * (255K / BUS_DC));

    if (ctx->init_samples) {
        ctx->init_samples--;
    } else {
        PWM_0_WriteCompare(u_pwm_1);
        PWM_1_WriteCompare(u_pwm_2);
        PWM_2_WriteCompare(u_pwm_3);
        PWM_3_WriteCompare(u_pwm_5);
        PWM_4_WriteCompare1(u_pwm_6);
        PWM_4_WriteCompare2(u_pwm_7);
        //PWM_5_WriteCompare1(u_pwm_0);
        //PWM_5_WriteCompare2(u_pwm_4);
    }
}

hal_comp_t pwm_dc_comp_struct = {
    .name      = "pwm_dc",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct pwm_dc_ctx_t),
    .pin_count = sizeof(struct pwm_dc_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

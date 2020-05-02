#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(step_dir);

HAL_PIN(pos_x);
HAL_PIN(pos_y);
HAL_PIN(pos_a);
HAL_PIN(pos_b);

HAL_PIN(enable);

struct step_dir_ctx_t {
    int offset_x;
    int offset_y;
    int offset_a;
    int offset_b;
    int running;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct step_dir_ctx_t *ctx = (struct step_dir_ctx_t *) ctx_ptr;
    //struct step_dir_pin_ctx_t *pins = (struct step_dir_pin_ctx_t *) pin_ptr;

    StepDir_0_Setup(2000);
    StepDir_1_Setup(2000);
    StepDir_2_Setup(2000);
    StepDir_3_Setup(2000);
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct step_dir_ctx_t *ctx      = (struct step_dir_ctx_t *) ctx_ptr;
    struct step_dir_pin_ctx_t *pins = (struct step_dir_pin_ctx_t *) pin_ptr;
    int count_x, count_y, count_a, count_b;
  
    count_x = StepDir_0_Read();
    count_y = StepDir_1_Read();
    count_a = StepDir_2_Read();
    count_b = StepDir_3_Read();

    if (!ctx->running) {
        if (count_x > 100 && count_x < 1900) {
            ctx->offset_x = count_x;
            ctx->offset_y = count_y;
            ctx->offset_a = count_a;
            ctx->offset_b = count_b;
            ctx->running = 1;
            PIN(enable) = 1K;
        } else return;
    }

    PIN(pos_x) = mod((accum) (count_x - ctx->offset_x) * (M_PI / 1000K));
    PIN(pos_y) = mod((accum) (count_y - ctx->offset_y) * (M_PI / 1000K));
    PIN(pos_a) = mod((accum) (count_a - ctx->offset_a) * (M_PI / 1000K));
    PIN(pos_b) = mod((accum) (count_b - ctx->offset_b) * (M_PI / 1000K));
}

hal_comp_t step_dir_comp_struct = {
    .name      = "step_dir",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct step_dir_ctx_t),
    .pin_count = sizeof(struct step_dir_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

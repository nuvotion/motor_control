#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(step_dir);

HAL_PIN(pos);

/*
struct step_dir_ctx_t {
};
*/

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct step_dir_ctx_t *ctx = (struct step_dir_ctx_t *) ctx_ptr;
    //struct step_dir_pin_ctx_t *pins = (struct step_dir_pin_ctx_t *) pin_ptr;

    StepDir_0_Setup(2000);
    StepDir_1_Setup(2000);
}

static void rt_func(float period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct step_dir_ctx_t *ctx      = (struct step_dir_ctx_t *) ctx_ptr;
    struct step_dir_pin_ctx_t *pins = (struct step_dir_pin_ctx_t *) pin_ptr;
    int count;
  
    count = StepDir_0_Read();

    PIN(pos) = mod(((float) count / 2000.0) * 2 * M_PI);
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
    .ctx_size  = 0, //sizeof(struct step_dir_ctx_t),
    .pin_count = sizeof(struct step_dir_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

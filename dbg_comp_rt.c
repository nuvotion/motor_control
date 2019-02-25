#include "hal.h"

HAL_COMP(testrt);

HAL_PIN(out);

extern uint64_t systime;
extern int rt_deadline_err;

static void rt_func(float period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct testrt_ctx_t *ctx      = (struct testrt_ctx_t *)ctx_ptr;
    struct testrt_pin_ctx_t *pins = (struct testrt_pin_ctx_t *)pin_ptr;

    PIN(out) = systime;
}

hal_comp_t testrt_comp_struct = {
    .name      = "testrt",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct testrt_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

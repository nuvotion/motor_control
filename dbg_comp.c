#include "hal.h"
#include "print.h"

HAL_COMP(dbg);

HAL_PIN(in);

extern void print(char *string);

static void nrt_func(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct dbg_ctx_t *ctx      = (struct dbg_ctx_t *)ctx_ptr;
    struct dbg_pin_ctx_t *pins = (struct dbg_pin_ctx_t *)pin_ptr;

    print("Input: ");
    print(print_float(PIN(in)));
    print("\r\n");
}

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    print("Hello world - init\r\n");
}

hal_comp_t dbg_comp_struct = {
    .name      = "dbg",
    .nrt       = nrt_func,
    .rt        = 0,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct dbg_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

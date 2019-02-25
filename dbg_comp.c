#include "hal.h"
#include "print.h"

HAL_COMP(test);

HAL_PIN(in);

extern void print(char *string);

static void nrt_func(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct test_ctx_t *ctx      = (struct test_ctx_t *)ctx_ptr;
    struct test_pin_ctx_t *pins = (struct test_pin_ctx_t *)pin_ptr;

    print("Hello world - testing from the hal nrt: ");
    print(print_float(PIN(in)));
    print("\r\n");
}

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    print("Hello world - init\r\n");
}

hal_comp_t test_comp_struct = {
    .name      = "test",
    .nrt       = nrt_func,
    .rt        = 0,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct test_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

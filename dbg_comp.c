#include "hal.h"
#include "print.h"
#include "angle.h"

HAL_COMP(dbg);

HAL_PIN(in0);
HAL_PIN(in1);
HAL_PIN(in2);
HAL_PIN(in3);
HAL_PIN(in4);
HAL_PIN(in5);
HAL_PIN(in6);

HAL_PIN(angle);

extern void print(char *string);

struct dbg_ctx_t {
    float angle;
};

static void nrt_func(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct dbg_ctx_t *ctx      = (struct dbg_ctx_t *)ctx_ptr;
    struct dbg_pin_ctx_t *pins = (struct dbg_pin_ctx_t *)pin_ptr;

    ctx->angle = ctx->angle + 0.03;

    print("iu: ");
    print(print_float(PIN(in0)));
    print(", iw: ");
    print(print_float(PIN(in1)));
    print(", d: ");
    print(print_float(PIN(in2)));
    print(", q: ");
    print(print_float(PIN(in3)));
    print(", u: ");
    print(print_float(PIN(in4)));
    print(", v: ");
    print(print_float(PIN(in5)));
    print(", w: ");
    print(print_float(PIN(in6)));
    print("\r\n");

    PIN(angle) = ctx->angle;
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
    .ctx_size  = sizeof(struct dbg_ctx_t),
    .pin_count = sizeof(struct dbg_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

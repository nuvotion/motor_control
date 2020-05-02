#include <project.h>
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
HAL_PIN(step);
HAL_PIN(step2);

HAL_PIN(error0);
HAL_PIN(error1);

extern void print(char *string);

struct dbg_ctx_t {
    accum angle;
    int count;
    int step;
    int init_samples;
    int enabled;
};

static void nrt_func(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct dbg_ctx_t *ctx      = (struct dbg_ctx_t *)ctx_ptr;
    struct dbg_pin_ctx_t *pins = (struct dbg_pin_ctx_t *)pin_ptr;

    print("in0: ");
    print(print_float(PIN(in0)));
    print(", in1: ");
    print(print_float(PIN(in1)));
    print(", in2: ");
    print(print_float(PIN(in2)));
    print(", in3: ");
    print(print_float(PIN(in3)));
    print(", in4: ");
    print(print_float(PIN(in4)));
    print("\n");

    if (PIN(error0) != 0K) LED_0_Write(0);
    if (PIN(error1) != 0K) LED_0_Write(0);
}

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct dbg_ctx_t *ctx      = (struct dbg_ctx_t *)ctx_ptr;

    ctx->init_samples = 25000*1; //5000*20;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct dbg_ctx_t *ctx      = (struct dbg_ctx_t *)ctx_ptr;
    struct dbg_pin_ctx_t *pins = (struct dbg_pin_ctx_t *)pin_ptr;

    if (ctx->init_samples) {
        ctx->init_samples--;
        return;
    }

    if (ctx->step) {
        ctx->angle = ctx->angle - 0.001K;
    }
    
    ctx->count++;
    if (ctx->count == 5000) {
        ctx->count = 0;
        ctx->step = !ctx->step;
    }

    PIN(angle) = mod(ctx->angle);
    PIN(step) = ctx->step ? 0.5K : 0K;
    PIN(step2) = ctx->step ? 100K : 0K;
}

hal_comp_t dbg_comp_struct = {
    .name      = "dbg",
    .nrt       = nrt_func,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct dbg_ctx_t),
    .pin_count = sizeof(struct dbg_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

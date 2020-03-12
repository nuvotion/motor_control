#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(vel);

HAL_PIN(pos_in);
HAL_PIN(pos_out);
HAL_PIN(vel);
HAL_PIN(w);
HAL_PIN(d);
HAL_PIN(lp);
HAL_PIN(torque);
HAL_PIN(vel_ff);

struct vel_ctx_t {
  accum last_acc;
  sat accum acc_sum;
  sat accum vel_sum;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  // struct vel_ctx_t * ctx = (struct vel_ctx_t *)ctx_ptr;
  struct vel_pin_ctx_t *pins = (struct vel_pin_ctx_t *)pin_ptr;

  PIN(w)  = 1000.0;
  PIN(d)  = 0.9;
  PIN(lp) = 50.0;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct vel_ctx_t *ctx      = (struct vel_ctx_t *)ctx_ptr;
  struct vel_pin_ctx_t *pins = (struct vel_pin_ctx_t *)pin_ptr;

  ctx->vel_sum += ctx->acc_sum * period;  // ff

  accum pos_error = minus(PIN(pos_in), ctx->vel_sum);
  accum w         = PIN(w);
  accum d         = PIN(d);
  accum lp        = LP_HZ(PIN(lp));
  accum acc       = PIN(torque);

  ctx->last_acc = acc * lp + (1K - lp) * ctx->last_acc;

  sat accum acc_ff = acc - ctx->last_acc;

  acc_ff += pos_error * w * w;

  ctx->acc_sum += acc_ff * period;

  PIN(vel) = ctx->acc_sum;

  accum vel_ff = 2K * d * w * pos_error;

  ctx->vel_sum += vel_ff * period;
  ctx->vel_sum = mod(ctx->vel_sum);

  PIN(pos_out)   = ctx->vel_sum;
}

hal_comp_t vel_comp_struct = {
    .name      = "vel",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct vel_ctx_t),
    .pin_count = sizeof(struct vel_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

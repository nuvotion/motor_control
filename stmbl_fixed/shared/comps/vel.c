#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(vel);

HAL_PIN(pos_in);
HAL_PIN(torque);
HAL_PIN(pos_out);
HAL_PIN(vel);

HAL_PIN(dbg);

struct vel_ctx_t {
  accum last_acc;
  sat accum acc_sum;
  sat accum vel_sum;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct vel_ctx_t *ctx      = (struct vel_ctx_t *)ctx_ptr;
  struct vel_pin_ctx_t *pins = (struct vel_pin_ctx_t *)pin_ptr;

  ctx->vel_sum += (ctx->acc_sum * period) + ACCUM_EPSILON;  // ff

  accum pos_error = minus(PIN(pos_in), ctx->vel_sum);
  accum acc       = PIN(torque) * period;

  const accum lp = LP_HZ(50.0, 0.0002);
  ctx->last_acc = acc * lp + (1K - lp) * ctx->last_acc;

  sat accum acc_ff = acc - ctx->last_acc;

  acc_ff += pos_error * 200K;

  ctx->acc_sum += acc_ff;

  PIN(vel) = ctx->acc_sum;

  ctx->vel_sum += pos_error * 0.36K;
  ctx->vel_sum = mod(ctx->vel_sum);

  PIN(pos_out)   = ctx->vel_sum;
}

hal_comp_t vel_comp_struct = {
    .name      = "vel",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct vel_ctx_t),
    .pin_count = sizeof(struct vel_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

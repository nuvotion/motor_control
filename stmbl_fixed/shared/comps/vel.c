#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "mac.h"
#include "constants.h"

HAL_COMP(vel);

HAL_PIN(pos_in);
HAL_PIN(torque);
HAL_PIN(pos_out);
HAL_PIN(vel);

HAL_PIN(dbg);

struct vel_ctx_t {
  accum last_acc;
  int64_t acc_sum;
  int64_t vel_sum;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct vel_ctx_t *ctx      = (struct vel_ctx_t *)ctx_ptr;
  struct vel_pin_ctx_t *pins = (struct vel_pin_ctx_t *)pin_ptr;

  ctx->vel_sum = mac(ctx->vel_sum, read_mac(ctx->acc_sum), period);

  accum pos_error = minus(PIN(pos_in), read_mac(ctx->vel_sum));
  accum acc_delta = PIN(torque) - ctx->last_acc;

  ctx->last_acc += acc_delta * (accum) VEL_ACC_LP;

  ctx->acc_sum = mac(ctx->acc_sum, acc_delta, period);
  ctx->acc_sum = mac(ctx->acc_sum, pos_error, (accum) VEL_ACC_FF_GAIN);

  ctx->vel_sum = mac(ctx->vel_sum, pos_error, (accum) VEL_VEL_FF_GAIN);
  ctx->vel_sum = mod_mac(ctx->vel_sum);

  PIN(pos_out) = read_mac(ctx->vel_sum);
  PIN(vel)     = read_mac(ctx->acc_sum);
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

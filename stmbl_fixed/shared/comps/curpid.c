#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "constants.h"
#include "mac.h"

HAL_COMP(curpid);

// current command
HAL_PIN(id_cmd);
HAL_PIN(iq_cmd);

// current feedback
HAL_PIN(id_fb);
HAL_PIN(iq_fb);

// voltage output
HAL_PIN(ud);
HAL_PIN(uq);

// Constants
HAL_PIN(kp);
HAL_PIN(kp_ki);

struct curpid_ctx_t {
  accum id_error_sum;
  accum iq_error_sum;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct curpid_ctx_t *ctx      = (struct curpid_ctx_t *)ctx_ptr;
  struct curpid_pin_ctx_t *pins = (struct curpid_pin_ctx_t *)pin_ptr;

  accum idc     = PIN(id_cmd);
  accum iqc     = PIN(iq_cmd);

  idc = LIMIT(idc, CURPID_MAX_CURRENT);
  iqc = LIMIT(iqc, CURPID_MAX_CURRENT);

  accum id_error = idc - PIN(id_fb);
  accum iq_error = iqc - PIN(iq_fb);

  accum ud = LIMIT(id_error * PIN(kp), (accum) CURPID_BUS_3PH);
  accum uq = LIMIT(iq_error * PIN(kp), (accum) CURPID_BUS_3PH);

  ctx->id_error_sum += mul_rnd(id_error, PIN(kp_ki));
  ctx->iq_error_sum += mul_rnd(iq_error, PIN(kp_ki));

  ctx->id_error_sum = id_error >= 0K ? 
      MIN(ctx->id_error_sum,  CURPID_BUS_3PH - ud) :
      MAX(ctx->id_error_sum, -CURPID_BUS_3PH - ud);
  ctx->iq_error_sum = iq_error >= 0K ? 
      MIN(ctx->iq_error_sum,  CURPID_BUS_3PH - uq) :
      MAX(ctx->iq_error_sum, -CURPID_BUS_3PH - uq);

  ud += ctx->id_error_sum;
  uq += ctx->iq_error_sum;

  PIN(ud) = ud;
  PIN(uq) = uq;
}

hal_comp_t curpid_comp_struct = {
    .name      = "curpid",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct curpid_ctx_t),
    .pin_count = sizeof(struct curpid_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

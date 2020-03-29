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

struct curpid_ctx_t {
  int64_t id_error_sum;
  int64_t iq_error_sum;
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

  ctx->id_error_sum = mac(ctx->id_error_sum, 
                            id_error, CURPID_X_KP * CURPID_X_KI);
  ctx->iq_error_sum = mac(ctx->iq_error_sum,
                            iq_error, CURPID_X_KP * CURPID_X_KI);

  accum ud = read_mac(ctx->id_error_sum);
  accum uq = read_mac(ctx->iq_error_sum);

  ud += id_error * ((accum) CURPID_X_KP);
  uq += iq_error * ((accum) CURPID_X_KP);

  uq = LIMIT(uq, (accum) CURPID_BUS_3PH); 
  ud = LIMIT(ud, (accum) CURPID_BUS_3PH); 

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

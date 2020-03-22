#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "constants.h"

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
  sat accum id_error_sum;
  sat accum iq_error_sum;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct curpid_ctx_t *ctx      = (struct curpid_ctx_t *)ctx_ptr;
  struct curpid_pin_ctx_t *pins = (struct curpid_pin_ctx_t *)pin_ptr;

  accum idc     = PIN(id_cmd);
  accum iqc     = PIN(iq_cmd);

  idc = LIMIT(idc, CURPID_MAX_CURRENT);
  iqc = LIMIT(iqc, CURPID_MAX_CURRENT);

  accum id = PIN(id_fb);
  accum iq = PIN(iq_fb);

  sat accum id_error = idc - id;
  sat accum iq_error = iqc - iq;

  ctx->id_error_sum += ((accum) (CURPID_X_KP * CURPID_X_KI)) * id_error;
  ctx->iq_error_sum += ((accum) (CURPID_X_KP * CURPID_X_KI)) * iq_error;

  sat accum ud = ((accum) CURPID_X_KP) * id_error + ctx->id_error_sum;
  sat accum uq = ((accum) CURPID_X_KP) * iq_error + ctx->iq_error_sum;

  uq = LIMIT(uq, (sat accum) CURPID_BUS_3PH); 
  ud = LIMIT(ud, (sat accum) CURPID_BUS_3PH); 

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

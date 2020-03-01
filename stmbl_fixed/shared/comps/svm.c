#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(svm);

//U V W inputs
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

//U V W outputs
HAL_PIN(su);
HAL_PIN(sv);
HAL_PIN(sw);

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  // struct svm_ctx_t * ctx = (struct svm_ctx_t *)ctx_ptr;
  struct svm_pin_ctx_t *pins = (struct svm_pin_ctx_t *)pin_ptr;

  accum offset;

  accum u = PIN(u);
  accum v = PIN(v);
  accum w = PIN(w);

  offset = MIN3(u, v, w);

  PIN(su) = u - offset;
  PIN(sv) = v - offset;
  PIN(sw) = w - offset;
}

hal_comp_t svm_comp_struct = {
    .name      = "svm",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct svm_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

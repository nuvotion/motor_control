#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(idq);

//d,q inputs
HAL_PIN(d);
HAL_PIN(q);

//rotor position
HAL_PIN(pos);

//U V W output
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct idq_pin_ctx_t *pins = (struct idq_pin_ctx_t *)pin_ptr;

  accum d = PIN(d);
  accum q = PIN(q);

  accum pos = PIN(pos);

  accum si = 0.0K;
  accum co = 0.0K;
  sincos_fast(pos, &si, &co);

  //inverse park transformation
  accum a = d * co - q * si;
  accum b = d * si + q * co;

  //inverse clarke transformation
  accum u, v, w;

  u = a;
  v = -a * 0.5K + b * 0.5K * M_SQRT3;
  w = -a * 0.5K - b * 0.5K * M_SQRT3;

  PIN(u) = u;
  PIN(v) = v;
  PIN(w) = w;
}

hal_comp_t idq_comp_struct = {
    .name      = "idq",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct idq_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(dq);

//U V W inputs
HAL_PIN(u);
HAL_PIN(w);

//rotor position
HAL_PIN(pos);

//d,q output
HAL_PIN(d);
HAL_PIN(q);

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct dq_pin_ctx_t *pins = (struct dq_pin_ctx_t *)pin_ptr;

  accum u = PIN(u);
  accum w = PIN(w);

  //clarke transformation
  accum a, b;

  a = u;
  b = -(2.0K*w + u) * (1.0K/M_SQRT3);

  accum pos = PIN(pos);

  accum si = 0.0K;
  accum co = 0.0K;
  sincos_fast(pos, &si, &co);

  //park transformation
  accum d = a * co + b * si;
  accum q = -a * si + b * co;

  PIN(d) = d;
  PIN(q) = q;
}

hal_comp_t dq_comp_struct = {
    .name      = "dq",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct dq_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

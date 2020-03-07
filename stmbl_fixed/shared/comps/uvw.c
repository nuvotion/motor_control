#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(uvw);

//u,v,w inputs
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

//rotor position output
HAL_PIN(pos);

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  // struct uvw_ctx_t * ctx = (struct uvw_ctx_t *)ctx_ptr;
  struct uvw_pin_ctx_t *pins = (struct uvw_pin_ctx_t *)pin_ptr;
  const accum t[8] = {
      0, 0, 2K*M_PI/3K,  M_PI/3K, 
           -2K*M_PI/3K, -M_PI/3K, -M_PI, 0};
  short u, v, w, idx;

  u = (short) PIN(u);
  v = (short) PIN(v);
  w = (short) PIN(w);

  idx = (w << 2) | (v << 1) | (u << 0);

  PIN(pos) = t[idx];
}

hal_comp_t uvw_comp_struct = {
    .name      = "uvw",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = 0,
    .pin_count = sizeof(struct uvw_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

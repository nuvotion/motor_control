#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(fb_switch);

HAL_PIN(polecount);

HAL_PIN(com_fb);

HAL_PIN(mot_abs_pos);
HAL_PIN(mot_polecount);
HAL_PIN(mot_offset);
HAL_PIN(mot_state);  // 0 = disabled, 1 = inc, 2 = start abs, 3 = abs

HAL_PIN(com_abs_pos);
HAL_PIN(com_polecount);
HAL_PIN(com_offset);

HAL_PIN(current_com_pos);

struct fb_switch_ctx_t {
  int current_com_pos;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct fb_switch_ctx_t *ctx      = (struct fb_switch_ctx_t *)ctx_ptr;
  //struct fb_switch_pin_ctx_t *pins = (struct fb_switch_pin_ctx_t *)pin_ptr;

  ctx->current_com_pos = 2;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct fb_switch_ctx_t *ctx      = (struct fb_switch_ctx_t *)ctx_ptr;
  struct fb_switch_pin_ctx_t *pins = (struct fb_switch_pin_ctx_t *)pin_ptr;

  accum mot_abs_pos   = PIN(mot_abs_pos);
  accum com_abs_pos   = PIN(com_abs_pos);

  accum mot_offset   = PIN(mot_offset);
  accum com_offset   = PIN(com_offset);

  if(PIN(mot_state) >= 2K) {
    ctx->current_com_pos = 1; // mot fb absolute
  }

  switch(ctx->current_com_pos) {
    case 2:
      PIN(com_fb) = mod((com_abs_pos + com_offset) * PIN(polecount) / PIN(com_polecount));
      break;

    case 1:
      PIN(com_fb) = mod((mot_abs_pos + mot_offset) * PIN(polecount) / PIN(mot_polecount));
      break;
  }
  PIN(current_com_pos) = ctx->current_com_pos;
}

hal_comp_t fb_switch_comp_struct = {
    .name      = "fb_switch",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct fb_switch_ctx_t),
    .pin_count = sizeof(struct fb_switch_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

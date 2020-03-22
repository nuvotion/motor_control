#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "constants.h"

HAL_COMP(fb_switch);

HAL_PIN(com_fb);

HAL_PIN(mot_abs_pos);
HAL_PIN(mot_state);  // 0 = disabled, 1 = inc, 2 = start abs, 3 = abs

HAL_PIN(com_abs_pos);

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

  if(PIN(mot_state) >= 2K) {
    ctx->current_com_pos = 1; // mot fb absolute
  }

  switch(ctx->current_com_pos) {
    case 2:
      PIN(com_fb) = mod((com_abs_pos + FB_COM_OFFSET) * FB_POLECOUNT / FB_COM_POLECOUNT);
      break;

    case 1:
      PIN(com_fb) = mod((mot_abs_pos + FB_MOT_OFFSET) * FB_POLECOUNT / FB_MOT_POLECOUNT);
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

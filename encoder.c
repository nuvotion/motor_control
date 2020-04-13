#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "constants.h"

HAL_COMP(encoder);

HAL_PIN(com_pos_x);
HAL_PIN(mot_pos_x);
HAL_PIN(com_pos_y);
HAL_PIN(mot_pos_y);

HAL_PIN(enable);

struct encoder_ctx_t {
    int index_offset_x;
    int index_found_x;
    int index_offset_y;
    int index_found_y;
    int running;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct encoder_ctx_t *ctx      = (struct encoder_ctx_t *) ctx_ptr;
    struct encoder_pin_ctx_t *pins = (struct encoder_pin_ctx_t *) pin_ptr;
    int count_x, idx_x, count_y, idx_y;
    accum com_abs_pos_x, mot_abs_pos_x, com_abs_pos_y, mot_abs_pos_y;
    const accum t[8] = {
        0, 0, 2K*M_PI/3K,  M_PI/3K, 
             -2K*M_PI/3K, -M_PI/3K, -M_PI, 0};

    if (PIN(enable) == 0K) return;

    if (!ctx->running) {
        QuadDec_0_Start();
        QuadDec_2_Start();
        ctx->running = 1;
    }
  
    idx_x = (QUAD1_U_Read() << 2) | (QUAD1_V_Read() << 1) | (QUAD1_W_Read() << 0);
    idx_y = (QUAD3_U_Read() << 2) | (QUAD3_V_Read() << 1) | (QUAD3_W_Read() << 0);

    com_abs_pos_x = t[idx_x];
    com_abs_pos_y = t[idx_y];

    if (!ctx->index_found_x &&
            QuadDec_0_ReadStatusRegister() & QuadDec_0_STATUS_CAPTURE) {
        count_x = QuadDec_0_ReadCapture() - 0x8000;
        count_x = (count_x <= 0) ? -count_x : 2000 - count_x;
        ctx->index_offset_x = count_x;
        ctx->index_found_x = 1;
    }

    if (!ctx->index_found_y &&
            QuadDec_2_ReadStatusRegister() & QuadDec_2_STATUS_CAPTURE) {
        count_y = QuadDec_2_ReadCapture() - 0x8000;
        count_y = (count_y <= 0) ? -count_y : 2000 - count_y;
        ctx->index_offset_y = count_y;
        ctx->index_found_y = 1;
    }

    count_x = QuadDec_0_ReadCounter() - 0x8000;
    count_x = (count_x <= 0) ? -count_x : 2000 - count_x;
    count_y = QuadDec_2_ReadCounter() - 0x8000;
    count_y = (count_y <= 0) ? -count_y : 2000 - count_y;

    mot_abs_pos_x = mod((accum) count_x * (M_PI / 1000K));
    mot_abs_pos_y = mod((accum) count_y * (M_PI / 1000K));

    if (!ctx->index_found_x) {
        PIN(com_pos_x) = mod((com_abs_pos_x + FB_COM_OFFSET) * (FB_POLECOUNT / FB_COM_POLECOUNT));
    } else {
        PIN(com_pos_x) = mod((mot_abs_pos_x + FB_MOT_OFFSET) * (FB_POLECOUNT / FB_MOT_POLECOUNT));
    }

    if (!ctx->index_found_y) {
        PIN(com_pos_y) = mod((com_abs_pos_y + FB_COM_OFFSET) * (FB_POLECOUNT / FB_COM_POLECOUNT));
    } else {
        PIN(com_pos_y) = mod((mot_abs_pos_y + FB_MOT_OFFSET) * (FB_POLECOUNT / FB_MOT_POLECOUNT));
    }

    count_x += ctx->index_offset_x;
    count_y += ctx->index_offset_y;

    PIN(mot_pos_x) = mod((accum) count_x * (M_PI / 1000K));
    PIN(mot_pos_y) = mod((accum) count_y * (M_PI / 1000K));
}

hal_comp_t encoder_comp_struct = {
    .name      = "encoder",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct encoder_ctx_t),
    .pin_count = sizeof(struct encoder_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

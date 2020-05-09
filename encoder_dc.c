#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"
#include "constants.h"

HAL_COMP(encoder_dc);

HAL_PIN(mot_pos_0);
HAL_PIN(mot_pos_1);
HAL_PIN(mot_pos_2);
HAL_PIN(mot_pos_3);

HAL_PIN(enable);

struct encoder_dc_ctx_t {
    int index_offset_0;
    int index_found_0;
    int index_offset_1;
    int index_found_1;
    int index_offset_2;
    int index_found_2;
    int index_offset_3;
    int index_found_3;
    int running;
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct encoder_dc_ctx_t *ctx      = (struct encoder_dc_ctx_t *) ctx_ptr;
    struct encoder_dc_pin_ctx_t *pins = (struct encoder_dc_pin_ctx_t *) pin_ptr;
    int count_0, count_1, count_2, count_3;

    if (PIN(enable) == 0K) return;

    if (!ctx->running) {
        QuadDec_0_Start();
        QuadDec_1_Start();
        QuadDec_2_Start();
        QuadDec_3_Start();
        ctx->running = 1;
    }
  
    if (!ctx->index_found_0 &&
            QuadDec_0_ReadStatusRegister() & QuadDec_0_STATUS_CAPTURE) {
        count_0 = QuadDec_0_ReadCapture() - 0x8000;
        count_0 = (count_0 <= 0) ? -count_0 : 1000 - count_0;
        ctx->index_offset_0 = count_0;
        ctx->index_found_0 = 1;
    }

    if (!ctx->index_found_1 &&
            QuadDec_1_ReadStatusRegister() & QuadDec_1_STATUS_CAPTURE) {
        count_1 = QuadDec_1_ReadCapture() - 0x8000;
        count_1 = (count_1 <= 0) ? -count_1 : 1000 - count_1;
        ctx->index_offset_1 = count_1;
        ctx->index_found_1 = 1;
    }

    if (!ctx->index_found_2 &&
            QuadDec_2_ReadStatusRegister() & QuadDec_2_STATUS_CAPTURE) {
        count_2 = QuadDec_2_ReadCapture() - 0x8000;
        count_2 = (count_2 <= 0) ? -count_2 : 1000 - count_2;
        ctx->index_offset_2 = count_2;
        ctx->index_found_2 = 1;
    }

    if (!ctx->index_found_3 &&
            QuadDec_3_ReadStatusRegister() & QuadDec_3_STATUS_CAPTURE) {
        count_3 = QuadDec_3_ReadCapture() - 0x8000;
        count_3 = (count_3 <= 0) ? -count_3 : 1000 - count_3;
        ctx->index_offset_3 = count_3;
        ctx->index_found_3 = 1;
    }

    count_0 = QuadDec_0_ReadCounter() - 0x8000;
    count_0 = (count_0 <= 0) ? -count_0 : 1000 - count_0;
    count_1 = QuadDec_1_ReadCounter() - 0x8000;
    count_1 = (count_1 <= 0) ? -count_1 : 1000 - count_1;
    count_2 = QuadDec_2_ReadCounter() - 0x8000;
    count_2 = (count_2 <= 0) ? -count_2 : 1000 - count_2;
    count_3 = QuadDec_3_ReadCounter() - 0x8000;
    count_3 = (count_3 <= 0) ? -count_3 : 1000 - count_3;

    count_0 += ctx->index_offset_0;
    count_1 += ctx->index_offset_1;
    count_2 += ctx->index_offset_2;
    count_3 += ctx->index_offset_3;

    PIN(mot_pos_0) = mod((accum) count_0 * (M_PI / 500K));
    PIN(mot_pos_1) = mod((accum) count_1 * (M_PI / 500K));
    PIN(mot_pos_2) = mod((accum) count_2 * (M_PI / 500K));
    PIN(mot_pos_3) = mod((accum) count_3 * (M_PI / 500K));
}

hal_comp_t encoder_dc_comp_struct = {
    .name      = "encoder_dc",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct encoder_dc_ctx_t),
    .pin_count = sizeof(struct encoder_dc_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

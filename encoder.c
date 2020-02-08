#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(encoder);

// U V W output
HAL_PIN(u);
HAL_PIN(v);
HAL_PIN(w);

HAL_PIN(mot_abs_pos);
HAL_PIN(mot_pos);
HAL_PIN(mot_state);
HAL_PIN(index_pos);

struct encoder_ctx_t {
    int index_offset;
    int index_found;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct encoder_ctx_t *ctx = (struct encoder_ctx_t *) ctx_ptr;
    struct encoder_pin_ctx_t *pins = (struct encoder_pin_ctx_t *) pin_ptr;

    QuadDec_0_Start();
    //QuadDec_2_Start();
    QuadDec_0_WriteCounter(0x8000);
    //QuadDec_2_WriteCounter(0x8000);

    PIN(mot_state) = 1.0;
}

static void rt_func(float period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct encoder_ctx_t *ctx      = (struct encoder_ctx_t *) ctx_ptr;
    struct encoder_pin_ctx_t *pins = (struct encoder_pin_ctx_t *) pin_ptr;
    int count;
  
    PIN(u) = QUAD1_W_Read();
    PIN(v) = QUAD1_V_Read();
    PIN(w) = QUAD1_U_Read();

    if (!ctx->index_found &&
            QuadDec_0_ReadStatusRegister() & QuadDec_0_STATUS_CAPTURE) {
        count = QuadDec_0_ReadCapture() - 0x8000;
        count = (count <= 0) ? -count : 2000 - count;
        ctx->index_offset = count;
        ctx->index_found = 1;
        PIN(index_pos) = count;
        PIN(mot_state) = 3.0;
    }

    count = QuadDec_0_ReadCounter() - 0x8000;
    count = (count <= 0) ? -count : 2000 - count;

    PIN(mot_abs_pos) = mod(((float) count / 2000.0) * 2 * M_PI);

    count += ctx->index_offset;

    PIN(mot_pos) = mod(((float) count / 2000.0) * 2 * M_PI);
}

hal_comp_t encoder_comp_struct = {
    .name      = "encoder",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct encoder_ctx_t),
    .pin_count = sizeof(struct encoder_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

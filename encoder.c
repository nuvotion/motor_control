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
HAL_PIN(mot_state);

/*
struct encoder_ctx_t {
};
*/

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct encoder_ctx_t *ctx = (struct encoder_ctx_t *) ctx_ptr;
    struct encoder_pin_ctx_t *pins = (struct encoder_pin_ctx_t *) pin_ptr;

    QuadDec_0_Start();
    //QuadDec_2_Start();

    PIN(mot_state) = 1.0;
}

static void rt_func(float period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    //struct encoder_ctx_t *ctx      = (struct encoder_ctx_t *) ctx_ptr;
    struct encoder_pin_ctx_t *pins = (struct encoder_pin_ctx_t *) pin_ptr;
    int count;
  
    PIN(u) = QUAD1_W_Read();
    PIN(v) = QUAD1_V_Read();
    PIN(w) = QUAD1_U_Read();

    count = QuadDec_0_GetCounter();
    if (count <= 0) {
        count = -count;
    } else {
        count = 2000 - count;
    }

    PIN(mot_abs_pos) = mod(((float) count / 2000.0) * 2 * M_PI);

    if (QuadDec_0_GetEvents() & 4) {
        PIN(mot_state) = 3.0;
    }
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
    .ctx_size  = 0, //sizeof(struct encoder_ctx_t),
    .pin_count = sizeof(struct encoder_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

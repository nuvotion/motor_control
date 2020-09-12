#include <project.h>
#include "hal.h"
#include "defines.h"
#include "angle.h"

HAL_COMP(ufm);

HAL_PIN(pos_x);
HAL_PIN(pos_y);
HAL_PIN(pos_a);
HAL_PIN(pos_b);
HAL_PIN(pos_c);
HAL_PIN(pos_d);

HAL_PIN(enable);

#define BUF_SIZE 32
#define NUM_AXIS 6
#define MAX_PKTS (BUF_SIZE/(NUM_AXIS+2))

struct ufm_ctx_t {
    uint8_t ufm_data[BUF_SIZE*2];
    int steps[NUM_AXIS];
    int filt[NUM_AXIS];
    int ufm_data_idx;
    int last_packet_id;
    int running;
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct ufm_ctx_t *ctx = (struct ufm_ctx_t *) ctx_ptr;
    //struct ufm_pin_ctx_t *pins = (struct ufm_pin_ctx_t *) pin_ptr;
    
    ctx->last_packet_id = -1;
    I2C_UFM_SetupDMA(BUF_SIZE*2, ctx->ufm_data);
}

static inline int is_start(uint8_t csr) {
    return ((csr & 9) == 9);
}

static inline int is_data(uint8_t csr) {
    return ((csr & 9) == 1);
}

static inline int is_end(uint8_t csr) {
    return ((csr & 33) == 32);
}

static inline int wrap(unsigned int i) {
    return i % BUF_SIZE;
}

static inline int find_packets(uint8_t *buf, int start, int *end) {
    int i, count, found;

    count = 0;
    found = 0;
    i = start;

    do {
        if (is_end(buf[i]) && count == 7) 
            end[found++] = i;

        if (found == MAX_PKTS) 
            break;

        if (is_start(buf[i]))
            count = 1;
        else if (is_data(buf[i]))
            count++;
        else    
            count = 0;

    } while ((i = wrap(++i)) != start);

    return found;
}

static inline void update_steps(struct ufm_ctx_t *ctx,
                                struct ufm_pin_ctx_t *pins, 
                                int *pkt_data,
                                int pkt) {
#define FILTER 1
    int i;

    if (ctx->running) {
        for (i = 0; i < NUM_AXIS; i++) {
#if FILTER
            if (pkt) {
                ctx->filt[i] += pkt_data[i];
            } else {
                ctx->steps[i] += pkt_data[i];
            }

            if (ctx->filt[i] > 0) {
                ctx->steps[i]++;
                ctx->filt[i]--;
            } else if (ctx->filt[i] < 0) {
                ctx->steps[i]--;
                ctx->filt[i]++;
            }
#else
            ctx->steps[i] += pkt_data[i];
#endif
        }
    } else if (pkt_data[0]) {
        ctx->running = 1;
        PIN(enable) = 1K;
    }
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct ufm_ctx_t *ctx      = (struct ufm_ctx_t *) ctx_ptr;
    struct ufm_pin_ctx_t *pins = (struct ufm_pin_ctx_t *) pin_ptr;

    uint8_t pkt_id;
    int i, j, found, pkt, end[MAX_PKTS];
    int pkt_data[NUM_AXIS];

    if (!(found = find_packets(ctx->ufm_data, ctx->ufm_data_idx, end))) return;

    ctx->ufm_data_idx = wrap(end[found-1] + 1);

    for (pkt = 0; pkt < found; pkt++) {
        /* Copy packet with buffer wrap and invalidate original */
        i = wrap(end[pkt] - (NUM_AXIS+1));
        pkt_id = ctx->ufm_data[i+BUF_SIZE]; 
        ctx->ufm_data[i] = 0;
        i = wrap(++i);
        for (j = 0; j < NUM_AXIS; j++, i = wrap(++i)) {
            pkt_data[j] = (int8_t) ctx->ufm_data[i+BUF_SIZE];
            ctx->ufm_data[i] = 0;
        }
        ctx->ufm_data[i] = 0;

        /* Parse packet */
        if ((ctx->last_packet_id >= 0) &&
                (pkt_id != ((ctx->last_packet_id + 1) & 255))) {
            // Error
        }

        update_steps(ctx, pins, pkt_data, pkt);

        ctx->last_packet_id = pkt_id;
    }

    PIN(pos_x) = (accum) mod((accum) ctx->steps[0] * (M_PI / 1000K));
    PIN(pos_y) = (accum) mod((accum) ctx->steps[1] * (M_PI / 1000K));
    PIN(pos_a) = (accum) mod((accum) ctx->steps[2] * (M_PI / 1000K));
    PIN(pos_b) = (accum) mod((accum) ctx->steps[3] * (M_PI / 1000K));
    PIN(pos_c) = (accum) mod((accum) ctx->steps[4] * (M_PI / 1000K));
    PIN(pos_d) = (accum) mod((accum) ctx->steps[5] * (M_PI / 1000K));
}

hal_comp_t ufm_comp_struct = {
    .name      = "ufm",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct ufm_ctx_t),
    .pin_count = sizeof(struct ufm_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};

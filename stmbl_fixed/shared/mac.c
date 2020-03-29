#include <stdfix.h>
#include <stdint.h>
#include <string.h>
#include "defines.h"

int64_t mac(int64_t acc, accum a, accum b) {
    int32_t x, y;
    int64_t dx, dy;
    memcpy(&x, &a, 4);
    memcpy(&y, &b, 4);
    dx = (int64_t) x;
    dy = (int64_t) y;
    acc += dx * dy;
    return acc;
}

accum read_mac(int64_t acc) {
    int32_t x;
    accum r;
    x = (int32_t) (acc >> 15);
    memcpy(&r, &x, 4);
    return r;
}

static inline int64_t cast_accum_mac(const accum val) {
    int int_val;
    int64_t long_val;
    memcpy(&int_val, &val, sizeof(int));
    long_val = (int64_t) int_val;
    return long_val << 15;
}

int64_t mod_mac(int64_t acc) {
    acc += cast_accum_mac(M_PI);
    acc %= cast_accum_mac(2K*M_PI);
    if (acc < 0) acc += cast_accum_mac(M_PI);
    else acc -= cast_accum_mac(M_PI);
    return acc;
}

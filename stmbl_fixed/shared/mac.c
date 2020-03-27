#include <stdfix.h>
#include <stdint.h>
#include <string.h>

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

int64_t mod_mac(int64_t acc) {
  acc += 3373259426L;
  acc %= 6746518852L;
  if (acc < 0) acc += 3373259426L;
  else acc -= 3373259426L;
  return acc;
}

#pragma once

#include <stdint.h>

#define FIX_32_PRECISSION 6 // bits for the non decimal part
typedef int32_t fix32_t;

inline fix32_t ftofix32(float f) {
  return (fix32_t)(f * (1 << (FIX_32_PRECISSION)));
}

inline float fix32tof(fix32_t f) { return (float)f / (1 << FIX_32_PRECISSION); }

inline fix32_t fix32_mul(fix32_t a, fix32_t b) {
  return (a * b) >> FIX_32_PRECISSION;
}

#define FIX_64_PRECISSION 10 // bits for the non decimal part
typedef int64_t fix64_t;

inline fix64_t ftofix64(float f) {
  return (fix64_t)(f * (1 << (FIX_64_PRECISSION)));
}

inline float fix64tof(fix64_t f) { return (float)f / (1 << FIX_64_PRECISSION); }

inline fix64_t fix64_mul(fix64_t a, fix64_t b) {
  return ((int64_t)a * b) >> FIX_64_PRECISSION;
}
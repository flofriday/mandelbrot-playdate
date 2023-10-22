#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <complex.h>

#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"

#include "debug.h"

#include "mandelbrot.h"

PlaydateAPI *pd;

void init_mandellib(PlaydateAPI *api) { pd = api; }

typedef struct {
  float start_x;
  float start_y;
  float step;
  uint8_t *frame;

} RenderParameters;

typedef struct {
  bool top;
  bool right;
  bool bottom;
  bool left;
} Precalculated;

#define MAX_ITERATIONS 64

inline static bool read_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  return 1 - ((frame[index / 8] >> (7 - (index % 8))) & 1);
}

inline static void draw_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  frame[index / 8] &= (uint8_t)(0xFF7F >> (index % 8));
  // frame[index / 8] ^= (uint8_t)(0x80 >> (index % 8));
}

inline static bool calc_inside(int c, int r, RenderParameters params) {
  float y0 = params.start_y + r * params.step + (params.step / 2.0f);
  float x0 = params.start_x + c * params.step + (params.step / 2.0f);

  float x2 = 0.0;
  float y2 = 0.0;
  float w = 0.0;

  for (int iterations = 0; iterations < MAX_ITERATIONS; iterations++) {
    float x = x2 - y2 + x0;
    float y = w - x2 - y2 + y0;
    x2 = x * x;
    y2 = y * y;
    w = (x + y) * (x + y);

    if (x2 + y2 > 4.0f)
      return false;
  }

  return true;
}

static float cnorm(float _Complex z) {
  return crealf(z) * crealf(z) + cimagf(z) * cimagf(z);
}

static float _Complex m_attractor(float _Complex w0, float _Complex c, int p) {
  float _Complex w = w0;
  for (int m = 0; m < MAX_ITERATIONS; ++m) {
    float _Complex z = w;
    float _Complex dz = 1;
    for (int i = 0; i < p; ++i) {
      dz = 2 * z * dz;
      z = z * z + c;
    }
    w = w - (z - w) / (dz - 1.0f);
  }
  return w;
}

static float estimate_interior_distance(float _Complex z0, float _Complex c,
                                        int p) {
  float _Complex z = z0;
  float _Complex dz = 1;
  float _Complex dzdz = 0;
  float _Complex dc = 0;
  float _Complex dcdz = 0;
  for (int m = 0; m < p; ++m) {
    dcdz = 2 * (z * dcdz + dz * dc);
    dc = 2 * z * dc + 1;
    dzdz = 2 * (dz * dz + z * dzdz);
    dz = 2 * z * dz;
    z = z * z + c;
  }
  return (1.0f - cnorm(dz)) / cabsf(dcdz + dzdz * dc / (1.0f - dz));
}

static float estimate_distance(float _Complex c) {
  // FIXME: Lot's of cabsf can be replaced as they are compared and we can
  // simply square the other side of the comparison
  float _Complex dc = 0;
  float _Complex z = 0;
  float m = INFINITY;
  int p = 0;
  for (int n = 1; n < MAX_ITERATIONS; ++n) {
    dc = 2 * z * dc + 1;
    z = z * z + c;
    if (cnorm(z) > 4)
      return 2.0f * cabsf(z) * logf(cabsf(z)) / cabsf(dc);
    if (cnorm(z) < m) {
      m = cnorm(z);
      p = n;
      float _Complex z0 = m_attractor(z, c, p);
      float _Complex w = z0;
      float _Complex dw = 1;
      for (int k = 0; k < p; ++k) {
        dw = 2 * w * dw;
        w = w * w + c;
      }
      if (cnorm(dw) <= 1)
        return -estimate_interior_distance(z0, c, p);
    }
  }
  return 0;
}

// uint8_t *debug_bitmap = NULL;

//  At the momment this might recalculate some pixels more than once, if it
//  detects that they are not uniform
static void subdivide_mandelbrot(int x, int y, int n, RenderParameters params) {

  // FIXME: the comparison is not right
  // float _Complex point = CMPLX(params.start_x + params.step * (x + n / 2.0f),
  //                             params.start_y + params.step * (y + n / 2.0f));
  float _Complex point = (float complex)(
      (float)(params.start_x + params.step * (x + n / 2.0f)) +
      I * (float)(params.start_y + params.step * (y + n / 2.0f)));
  float distance = estimate_distance(point);
  float to_corner = ((n / 2.0f + 1.0f) * params.step) * 1.41421f * 1.5f;

  // flood-fill
  if (distance > to_corner) {
    // debug_draw_rect(debug_bitmap, x, y, n, n);
    return;
  }
  if (fabsf(distance) > to_corner) {
    // debug_draw_rect(debug_bitmap, x, y, n, n);
    for (int iy = y; iy < y + n; iy++) {
      for (int ix = x; ix < x + n; ix++) {
        draw_pixel(params.frame, ix, iy);
      }
    }
    return;
  }

  // Exit condition
  if (n <= 10) {
    for (int yi = y; yi < y + n; yi++) {
      for (int xi = x; xi < x + n; xi++) {
        bool is_black = calc_inside(xi, yi, params);
        if (is_black)
          draw_pixel(params.frame, xi, yi);
      }
    }
    return;
  }

  // Subdivide
  subdivide_mandelbrot(x, y, n / 2, params);
  subdivide_mandelbrot(x + n / 2, y, n / 2, params);
  subdivide_mandelbrot(x, y + n / 2, n / 2, params);
  subdivide_mandelbrot(x + n / 2, y + n / 2, n / 2, params);
}

int render_mandelbrot(lua_State *L) {
  float start_x = pd->lua->getArgFloat(1);
  float start_y = pd->lua->getArgFloat(2);
  float stop_x = pd->lua->getArgFloat(3);
  float stop_y = pd->lua->getArgFloat(4);
  float step = (stop_x - start_x) / LCD_COLUMNS;

  // FIXME: Polishing idea: Don't use subdivide if too far scrolled out.

  // pd->graphics->getBitmapData(pd->graphics->getDebugBitmap(), NULL, NULL,
  // NULL,
  //                            NULL, &debug_bitmap);

  RenderParameters params = {
      start_x,
      start_y,
      step,
      pd->graphics->getFrame(),
  };

  // FIXME: Test if smaller numbers would work better and with less graphical
  // glitches, this should not be an issue with distance estimator.
  for (int y = 0; y < LCD_ROWS; y += 80) {
    for (int x = 0; x < LCD_COLUMNS; x += 80) {
      subdivide_mandelbrot(x, y, 80, params);
    }
  }

  return 0;
}
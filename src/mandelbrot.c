#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"

#include "debug.h"

#include "mandelbrot.h"

PlaydateAPI *playdate;

void init_mandellib(PlaydateAPI *api) { playdate = api; }

typedef struct {
  float start_x;
  float start_y;
  float step_x;
  float step_y;
  uint8_t *frame;

} RenderParameters;

typedef struct {
  bool top;
  bool right;
  bool bottom;
  bool left;
} Precalculated;

#define MAX_ITERATIONS 64

inline bool read_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  return 1 - ((frame[index / 8] >> (7 - (index % 8))) & 1);
}

inline void draw_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  frame[index / 8] &= (uint8_t)(0xFF7F >> (index % 8));
  // frame[index / 8] ^= (uint8_t)(0x80 >> (index % 8));
}

inline bool calc_inside(int c, int r, RenderParameters params) {
  float y0 = params.start_y + r * params.step_y + (params.step_y / 2.0f);
  float x0 = params.start_x + c * params.step_x + (params.step_x / 2.0f);

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

float cnorm(float _Complex z) {
  return crealf(z) * crealf(z) + cimagf(z) * cimagf(z);
}

float estimate_interior_distance(float _Complex z0, float _Complex c, int p) {
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

float estimate_distance(float _Complex c) {
  float _Complex dc = 0;
  float _Complex z = 0;
  float m = 1.0 / 0.0;
  int p = 0;
  for (int n = 1; n <= MAX_ITERATIONS; ++n) {
    dc = 2 * z * dc + 1;
    z = z * z + c;
    if (cabsf(z) > 2)
      return 2.0f * cabsf(z) * logf(cabsf(z)) / cabsf(dc);
    if (cabsf(z) < m) {
      m = cabsf(z);
      p = n;
      // float _Complex z0 = m_attractor(z, c, p);
      float _Complex z0 = 0;
      float _Complex w = z0;
      float _Complex dw = 1;
      for (int k = 0; k < p; ++k) {
        dw = 2 * w * dw;
        w = w * w + c;
      }
      if (cabsf(dw) <= 1)
        return estimate_interior_distance(z0, c, p);
    }
  }
  return 0;
}

uint8_t *debug_bitmap = NULL;

//  At the momment this might recalculate some pixels more than once, if it
//  detects that they are not uniform
void subdivide_mandelbrot(int x, int y, int n, RenderParameters params,
                          Precalculated precalc) {

  // FIXME: There is a extreme possibility for an optimization, where we don't
  // need to calculate the border.
  // We take the point inside the cell and calculate it's distance estimator.
  // if it is farther away than the corners of the cell we don't have to trace
  // the border and can color the cell white
  // There seems to be a different distance estimator for inside of the
  // mandelbrot which would save even more time.
  // If the distance estimator is smaller than half the width of the cell we
  // know that there is definitely a structure inside the cell which would also
  // reduce the graphical glitches.

  // FIXME: the comparison is not right
  float _Complex point =
      CMPLXF(params.start_x + params.step_x * (x + n / 2.0f),
             params.start_y + params.step_y * (y + n / 2.0f));
  if (estimate_distance(point) > n * params.step_x) {
    debug_draw_rect(debug_bitmap, x, y, n, n);
  }

  // Calculate the border
  int c = x - (x == 0 ? 0 : 1);
  int r = y - (y == 0 ? 0 : 1);

  bool first =
      !precalc.top ? calc_inside(c, r, params) : read_pixel(params.frame, c, r);
  bool subdivide = false;

  // Calculate the top
  if (!precalc.top) {
    for (int i = 0; i < n; i++) {
      bool is_inside = calc_inside(x + i, r, params);
      if (is_inside != first) {
        subdivide = true;
      }
      if (is_inside) {
        draw_pixel(params.frame, x + i, r);
      }
    }
  }

  // Calculate the right or check it
  if (!precalc.right) {
    for (int i = 0; i < n - 1; i++) {
      bool is_inside = calc_inside(x + n - 1, y + i, params);
      if (is_inside != first) {
        subdivide = true;
      }
      if (is_inside) {
        draw_pixel(params.frame, x + n - 1, y + i);
      }
    }
  }

  // Calculate the bottom
  if (!precalc.bottom) {
    for (int i = 0; i < n; i++) {
      bool is_inside = calc_inside(x + i, y + n - 1, params);
      if (is_inside != first) {
        subdivide = true;
      }
      if (is_inside) {
        draw_pixel(params.frame, x + i, y + n - 1);
      }
    }
  }

  // Calculate the left or check it
  if (!precalc.left) {
    for (int i = 0; i < n; i++) {
      bool is_inside = calc_inside(c, y + i, params);
      if (is_inside != first) {
        subdivide = true;
      }
      if (is_inside) {
        draw_pixel(params.frame, c, y + i);
      }
    }
  }

  // Check precalculated if subdivision is necessary
  if (!subdivide) {
    // The order here is genius as in the first iterations it checks all the
    // 4 corners which are more likely to have different colors
    for (int i = 0; i < n; i++) {
      if (precalc.top && read_pixel(params.frame, x + i, r) != first) {
        subdivide = true;
        break;
      }

      if (precalc.right &&
          read_pixel(params.frame, x + n - 1, y + i) != first) {
        subdivide = true;
        break;
      }

      if (precalc.bottom &&
          read_pixel(params.frame, x + n - 1 - i, y + n - 1) != first) {
        subdivide = true;
        break;
      }

      if (precalc.left && read_pixel(params.frame, c, y + n - 1 - i) != first) {
        subdivide = true;
        break;
      }
    }
  }

  // Subdivide if necessary
  if (subdivide) {
    // Exit condition
    if (n < 10) {
      for (int yi = y == 0 ? 1 : 0; yi < n - 1; yi++) {
        for (int xi = x == 0 ? 1 : 0; xi < n - 1; xi++) {
          bool is_black = calc_inside(x + xi, y + yi, params);
          if (is_black)
            draw_pixel(params.frame, x + xi, y + yi);
        }
      }
      return;
    }

    // top left, top right, bottom left, bottom right
    Precalculated tl_precalc = {true, false, false, true};
    Precalculated tr_precalc = {true, true, false, true};
    Precalculated bl_precalc = {true, false, true, true};
    Precalculated br_precalc = {true, true, true, true};
    subdivide_mandelbrot(x, y, n / 2, params, tl_precalc);
    subdivide_mandelbrot(x + n / 2, y, n / 2, params, tr_precalc);
    subdivide_mandelbrot(x, y + n / 2, n / 2, params, bl_precalc);
    subdivide_mandelbrot(x + n / 2, y + n / 2, n / 2, params, br_precalc);
    return;
  }

  // Floodfill the rectangle inside the border
  if (!first)
    return;

  for (int iy = r + 1; iy < y + n - 1; iy++) {
    for (int ix = c + 1; ix < x + n - 1; ix++) {
      draw_pixel(params.frame, ix, iy);
    }
  }
}

int render_mandelbrot(lua_State *L) {
  float start_x = playdate->lua->getArgFloat(1);
  float start_y = playdate->lua->getArgFloat(2);
  float stop_x = playdate->lua->getArgFloat(3);
  float stop_y = playdate->lua->getArgFloat(4);
  float step_x = (stop_x - start_x) / LCD_COLUMNS;
  float step_y = (stop_y - start_y) / LCD_ROWS;

  // FIXME: Polishing idea: Don't use subdivide if too far scrolled out.

  playdate->graphics->getBitmapData(playdate->graphics->getDebugBitmap(), NULL,
                                    NULL, NULL, NULL, &debug_bitmap);

  RenderParameters params = {
      start_x, start_y, step_x, step_y, playdate->graphics->getFrame(),
  };

  // FIXME: Test if smaller numbers would work better and with less graphical
  // glitches, this should not be an issue with distance estimator.
  for (int y = 0; y < LCD_ROWS; y += 80) {
    for (int x = 0; x < LCD_COLUMNS; x += 80) {
      Precalculated precalc = {y != 0, false, false, x != 0};
      subdivide_mandelbrot(x, y, 80, params, precalc);
    }
  }

  return 0;
}
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"

PlaydateAPI *playdate;

int render_mandelbrot(lua_State *);
int get_build_time(lua_State *);

const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont *font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t arg) {
  (void)arg; // arg is currently only used for event = kEventKeyPressed

  if (event == kEventInit) {
    playdate = pd;

    const char *err;
    font = pd->graphics->loadFont(fontpath, &err);

    if (font == NULL)
      pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__,
                        fontpath, err);
  } else if (event == kEventInitLua) {
    const char *err;
    pd->lua->addFunction(render_mandelbrot, "drawNativeMandelbrot", &err);
    pd->lua->addFunction(get_build_time, "getBuildTime", &err);
  }

  return 0;
}

int get_build_time(lua_State *L) {
  playdate->lua->pushString(__TIMESTAMP__);
  return 1;
}

typedef struct {
  float start_x;
  float start_y;
  float step_x;
  float step_y;
  uint8_t *frame;

} RenderParameters;

#define MAX_ITERATIONS 64

inline void draw_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  frame[index / 8] &= (uint8_t)(0xFF7F >> (index % 8));
}

inline bool read_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  return frame[index / 8] & (0x80 >> (index % 8));
}

inline bool calc_inside(int c, int r, RenderParameters params) {
  float y0 = params.start_y + r * params.step_y + (params.step_y / 2.0f);
  float x0 = params.start_x + c * params.step_x + (params.step_x / 2.0f);

  float x2 = 0.0;
  float y2 = 0.0;
  float w = 0.0;

  int iterations = 0;
  for (; iterations < MAX_ITERATIONS; iterations++) {
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

// At the momment this might recalculate some pixels more than once, if it
// detects that they are not uniform
void subdivide_mandelbrot(int x, int y, int n, RenderParameters params) {

  // Check if the border is the same color
  bool first = calc_inside(x, y, params);
  bool subdivide = false;
  for (int i = 0; i < n - 1; i++) {
    // top, right, bottom, left
    bool t = calc_inside(x + i, y, params);
    if (t != first) {
      subdivide = true;
      break;
    }

    bool r = calc_inside(x + n - 1, y + i, params);
    if (r != first) {
      subdivide = true;
      break;
    }

    bool b = calc_inside(x + n - 1 - i, y + n - 1, params);
    if (b != first) {
      subdivide = true;
      break;
    }

    bool l = calc_inside(x, y + n - 1 - i, params);
    if (l != first) {
      subdivide = true;
      break;
    }
  }

  // Subdivide if necessary
  if (subdivide) {

    // Exit condition
    if (n < 10) {
      for (int yi = 0; yi < n; yi++) {
        for (int xi = 0; xi < n; xi++) {
          bool is_black = calc_inside(x + xi, y + yi, params);
          if (is_black)
            draw_pixel(params.frame, x + xi, y + yi);
        }
      }
      return;
    }

    // top left, top right, bottom left, bottom right
    subdivide_mandelbrot(x, y, n / 2, params);
    subdivide_mandelbrot(x + n / 2, y, n / 2, params);
    subdivide_mandelbrot(x, y + n / 2, n / 2, params);
    subdivide_mandelbrot(x + n / 2, y + n / 2, n / 2, params);
    return;
  }

  // Floodfill
  if (!first)
    return;

  for (int iy = 0; iy < n; iy++) {
    for (int ix = 0; ix < n; ix++) {
      draw_pixel(params.frame, x + ix, y + iy);
    }
  }
}

int render_mandelbrot(lua_State *L) {
  float start_x = -2.5;
  float start_y = 1.0;
  float stopx = 1.0;
  float stopy = -1.0;
  float step_x = (stopx - start_x) / LCD_COLUMNS;
  float step_y = (stopy - start_y) / LCD_ROWS;

  RenderParameters params = {
      start_x, start_y, step_x, step_y, playdate->graphics->getFrame(),
  };

  for (int y = 0; y < LCD_ROWS; y += 80) {
    for (int x = 0; x < LCD_COLUMNS; x += 80) {
      subdivide_mandelbrot(x, y, 80, params);
    }
  }

  return 0;
}
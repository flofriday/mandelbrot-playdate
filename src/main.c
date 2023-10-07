#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"

PlaydateAPI *playdate;

int render_mandelbrot(lua_State *);

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
  }

  return 0;
}

#define MAX_ITERATIONS 64

inline void draw_pixel(uint8_t *frame, int x, int y) {
  int index = x + y * (LCD_COLUMNS + 2 * 8);
  frame[index / 8] ^= 0x80 >> (index % 8);
}

inline int calc_iterations(int c, int r, float start_x, float start_y,
                           float step_x, float step_y) {
  float y0 = start_y + r * step_y;
  float x0 = start_x + c * step_x;

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
      return iterations;
  }

  return iterations;
}

int render_mandelbrot(lua_State *L) {
  float start_x = -2.5;
  float start_y = 1.0;
  float stopx = 1.0;
  float stopy = -1.0;
  float step_x = (stopx - start_x) / LCD_COLUMNS;
  float step_y = (stopy - start_y) / LCD_ROWS;

  playdate->system->logToConsole("Version: %s", __TIMESTAMP__);

  uint8_t *frame = playdate->graphics->getFrame();

  for (int r = 0; r < LCD_ROWS; r++) {
    for (int c = 0; c < LCD_COLUMNS; c++) {
      int iterations = calc_iterations(c, r, start_x, start_y, step_x, step_y);

      if (iterations == MAX_ITERATIONS)
        draw_pixel(frame, c, r);
    }
  }

  return 0;
}
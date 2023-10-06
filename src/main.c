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

int render_mandelbrot(lua_State *L) {
  float startx = -2.5;
  float starty = 1.0;
  float stopx = 1.0;
  float stopy = -1.0;
  int height = playdate->display->getHeight();
  int width = playdate->display->getWidth();
  float xStep = (stopx - startx) / width;
  float yStep = (stopy - starty) / height;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float cre = startx + x * xStep;
      float cim = starty + y * yStep;
      float zre = 0.0;
      float zim = 0.0;

      int iterations = 0;
      for (; iterations < MAX_ITERATIONS; iterations++) {
        float old_zre = zre;
        zre = (zre * zre - zim * zim) + cre;
        zim = (2.0f * old_zre * zim) + cim;

        if (zre * zre + zim * zim > 4.0f)
          break;
      }

      if (iterations == MAX_ITERATIONS)
        playdate->graphics->drawRect(x, y, 1, 1, kColorBlack);
    }
  }
  return 0;
}
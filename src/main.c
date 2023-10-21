#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mandelbrot.h"
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
    pd->system->logToConsole("Build Version: %s", __TIMESTAMP__);

    playdate = pd;
    init_mandellib(pd);

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

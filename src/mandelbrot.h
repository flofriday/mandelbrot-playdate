#pragma once

#include "pd_api.h"
#include "pd_api/pd_api_lua.h"

void init_mandellib(PlaydateAPI *api);
int render_mandelbrot(lua_State *L);
#ifndef _LUASTATE_H
#define _LUASTATE_H

#include <lapi.h>
#include <lauxlib.h>

//SET UP LUA State AS A GLOBAL
lua_State* L = luaL_newstate();

#endif
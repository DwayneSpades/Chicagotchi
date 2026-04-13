#ifndef _LUASTATE_H
#define _LUASTATE_H

#include <lapi.h>
#include <lauxlib.h>

//SET UP LUA State AS A GLOBAL
lua_State* L = luaL_newstate();

void lua_pcall_custom(lua_State* lState, int nargs, int nresults, int errfunc) {
    if (lua_pcall(lState, nargs, nresults, errfunc) != LUA_OK) {
        Serial.print("Lua error: ");
        Serial.println(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}
#endif
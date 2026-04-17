#ifndef _LUAUTIL_H
#define _LUAUTIL_H

#include <lua.h>
#include "Arduino.h"

namespace luaUtil {
    void printTypeAndValue(lua_State* L, int idx);
}

#endif
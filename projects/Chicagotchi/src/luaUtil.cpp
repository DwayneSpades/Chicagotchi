#include "luaUtil.h"

void luaUtil::printTypeAndValue(lua_State* L, int idx) {
    int t = lua_type(L, idx);
    const char* tName = lua_typename(L, t);

    Serial.print("type: ");
    Serial.print(tName);
    Serial.print(", value: ");
    switch (t) {
        case LUA_TNIL:
            Serial.print("nil");
            break;
        case LUA_TTABLE:
            Serial.print("{...}");
            break;
        case LUA_TNUMBER:
            Serial.print(lua_tonumber(L, idx));
            break;
        case LUA_TBOOLEAN:
            Serial.print(lua_toboolean(L, idx));
            break;
        case LUA_TSTRING:
            Serial.print(lua_tostring(L, idx));
            break;
        case LUA_TFUNCTION:
            Serial.print("function(\"...\")");
            break;
        default:
            Serial.print("Err: Unsupported type '");
            Serial.print(tName);
            Serial.print("'");
        break;
    }
    Serial.println("");
}
#include "packet.h"

#include <cstring>
#include <Arduino.h>

#include "..\luaUtil.h"

size_t my_strlen_s(const char* buf, int max) {
    for (size_t i = 0; i < max; i++){
        if (*buf == 0x0) {
            return i+1;
        }

        buf++;
    }

    return max;
}

packet::packet(const uint8_t* src_addr) {
    memcpy(sender_addr, src_addr, ESP_NOW_ETH_ALEN);
}

packet::packet(const uint8_t* src_addr, const uint8_t* buf, int len) {
    memcpy(sender_addr, src_addr, ESP_NOW_ETH_ALEN);
    data.insert(data.end(), buf, buf+len);
}

bool packet::serialize(uint16_t packetId, lua_State* L, int idx) {
    pushPacketId(packetId);
    processLuaTable(L, idx, 0);
}

bool packet::deserialize(lua_State* L) {
    const uint8_t* buf = data.data();
    size_t len = data.size();

    if (len == 0) {
        Serial.println("Abort -- no data!");
        return false;
    }

    int safety = 0;
    while (lua_type(L, -1) == LUA_TSTRING) {
        if (safety++ > 100) {
            Serial.println("whoops");
            return false;
        }
        Serial.println("Error?: ");
        Serial.print("\t");
        Serial.println(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    if (lua_type(L, 1) != LUA_TFUNCTION) {
        Serial.print("Abort! unexpected non-function value on the stack at luaStack:1: ");
        luaUtil::printTypeAndValue(L, 1);
        Serial.println("");

        return false;
    }

#if DBG_SER
    Serial.print("addr: ");
    for (int i = 0; i < 6; i++){
        Serial.print("0x");
        Serial.print(sender_addr[i], HEX);
        Serial.print(", ");
    }
    Serial.println("");
#endif

    // push the dest addr
    lua_pushlstring(L, (const char*)sender_addr, ESP_NOW_ETH_ALEN);

#if DBG_SER
    Serial.println("post push string");
    Serial.print("pre bit bang: len=");
    Serial.print(len);
    Serial.print(", buf[0]=");
    Serial.print(buf[0]);
    Serial.print(", buf[1]=");
    Serial.print(buf[1]);
    Serial.print(", buf[1] << 8 =");
    Serial.println(buf[1] << 8);
#endif

    // packetId is always first in the buffer
    uint16_t packetId = buf[0] | (buf[1] << 8);
    
    lua_pushnumber(L, (double)packetId);
    int i = 2;

#if DBG_SER
    Serial.print("Packet id: ");
    Serial.println(packetId);
#endif

    bool isKey = true; // false = isValue
    int tableCount = 0;

#if DBG_SER
    Serial.println("packet::deserialize!");
    printLuaStack(L);
#endif
    for (; i < len; i++) {
        switch(buf[i]) {
            // Primitives
            case packet_stamp::nil:
#if DBG_SER
                Serial.println("\t- packet_stamp::nil");
#endif
                if (isKey) {
                    Serial.println("Abort! nils cannot be keys.");
                    return false;
                } else {
                    lua_pushnil(L);
                }

#if DBG_SER
                printLuaStack(L);
#endif
                if (!isKey) lua_settable(L, -3);
                break;
            case packet_stamp::boolean:
#if DBG_SER
                Serial.println("\t- packet_stamp::boolean");
#endif
                i++;
                lua_pushboolean(L, (int)buf[i]);
#if DBG_SER
                printLuaStack(L);
#endif
                if (!isKey) lua_settable(L, -3);
                break;
            case packet_stamp::number:
#if DBG_SER
                Serial.println("\t- packet_stamp::number");
#endif
                // todo: copy eight bytes
                lua_pushnumber(L, (double)buf[i]);
                i += sizeof(double);
#if DBG_SER
                printLuaStack(L);
#endif
                if (!isKey) lua_settable(L, -3);
                break;
            case packet_stamp::string:
            {
                i++;
                uint16_t size = buf[i] | (buf[i+1] << 8);
                i += sizeof(uint16_t);
                char strbuf[MAX_STR_SIZE];
                memcpy(strbuf, buf + i, sizeof(char) * size);
                strbuf[size - 1] = 0x0; // jic
#if DBG_SER
                Serial.print("\t- packet_stamp::string: size=");
                Serial.print(size);
                Serial.print(", ");
                Serial.print(strbuf);
                Serial.println("");
#endif
                lua_pushstring(L, strbuf);
                i += size - 1;

#if DBG_SER
                printLuaStack(L);
#endif
                if (!isKey) lua_settable(L, -3);
            }
                break;

            // Tables
            case packet_stamp::table_begin:
#if DBG_SER
                Serial.println("\t- packet_stamp::table_begin");
#endif
                if (isKey && tableCount > 0) {
                    Serial.println("Abort! Tables cannot be keys.");
                    return false;
                } else {
                    tableCount++;
                    lua_newtable(L);
                }
                break;
            case packet_stamp::table_end:
#if DBG_SER
                Serial.print("\t- packet_stamp::table_end");
#endif
                tableCount--;
                if (tableCount > 0) {
                    lua_settable(L, -3);
#if DBG_SER
                    Serial.print("\t- settable / clear key");
#endif
                }

                Serial.println("");
                break;

            // Key/Value
            case packet_stamp::table_key:
#if DBG_SER
                Serial.println("\t- packet_stamp::table_key");
#endif
                isKey = true;
                break;
                
            case packet_stamp::table_value:
#if DBG_SER
                Serial.println("\t- packet_stamp::table_value");
#endif
                isKey = false;
                break;
        }
        
#if DBG_SER
        printLuaStack(L);
#endif
    }

    if (tableCount > 0) {
        Serial.print("Abort! somehow ended up with a tableCount > 0: ");
        Serial.print(tableCount);
        Serial.println("");
        return false;
    }

#if DBG_SER
    Serial.println("Successful deserialization!!!");
    printLuaStack(L);
#endif
    return true;
}

bool packet::processLuaTable(lua_State* L, int idx, int depth) {
    pushTableBegin();

    // dont totally understand this but it starts table iteration
    lua_pushnil(L);

    // push keys onto the stack -- i.e.
    // 1 (--): param1
    // 2 (-2): param1.keys[0]
    // 3 (-1): param1.values[0]
    while (lua_next(L, idx) != 0) {
#if DBG_SER
        indent();
        Serial.println("packet::serialize: nextloop");

        indent();  
        printLuaStack(L);
#endif
        if (pushLuaValue(L, -2, true)) { // key
            pushLuaValue(L, -1, false); // value

            int valueT = lua_type(L, -1);
            if (valueT == LUA_TTABLE) {
#if DBG_SER
                indent();
                Serial.println("{");
                tab++;
#endif

                if (depth > MAX_TABLE_DEPTH) {
                    Serial.print("Abort! exceeded max table depth: ");
                    Serial.print(MAX_TABLE_DEPTH);
                    Serial.println("");
                    return false;
                }
                if (!processLuaTable(L, lua_gettop(L), depth++)) {
                    return false;
                }
#if DBG_SER
                tab--;
                indent();
                Serial.println("}");
#endif
            } else {
#if DBG_SER
                indent();
#endif
            }
        }

        lua_pop(L, 1);
    }

    pushTableEnd();
    return true;
}

bool packet::pushLuaValue(lua_State* L, int idx, bool isKey) {
    if (isKey) {
        pushTableKey();
    } else {
        pushTableValue();
    }

#if DBG_SER
    luaUtil::printTypeAndValue(L, idx);
#endif

    int valueT = lua_type(L, idx);
    switch (valueT) {
        case LUA_TNIL:
            pushNil();
            break;
        case LUA_TTABLE:
            // skip
            if (isKey) {
                Serial.println("Err: Tables as keys are not supported.'");
                return false;
            }
            break;
        case LUA_TNUMBER:
            pushNumber(lua_tonumber(L, idx));
            break;
        case LUA_TBOOLEAN:
            pushNumber(lua_toboolean(L, idx));
            break;
        case LUA_TSTRING:
            pushString(lua_tostring(L, idx)); // <- this is cursed -- might be brittle...
            break;
        default:
            pushNil();
            Serial.print("Err: Unsupported type '");
            Serial.print(lua_typename(L, valueT));
            Serial.println("'");
            return false;
        break;
    }

    return true;
}

void packet::pushPacketId(uint16_t id) {
    if (data.size() > 0) {
        Serial.print("Err -- packetId should be the first item! Attempted to push id: ");
        Serial.print(id);
        Serial.println("");
        return;
    }

    data.insert(data.end(), (uint8_t*)&id, (uint8_t*)(&id + 1));
}

void packet::pushNil() {
    data.push_back(packet_stamp::nil);
}

void packet::pushNumber(double value) {
    data.push_back(packet_stamp::number);
    data.insert(data.end(), (uint8_t*)&value, (uint8_t*)(&value + 1));
}

void packet::pushBoolean(bool value) {
    data.push_back(packet_stamp::boolean);
    data.push_back((uint8_t)value);
}

void packet::pushString(const char* str) {
    packet_str_data str_data;
    size_t size = my_strlen_s(str, MAX_STR_SIZE);
    
    // was copying a buffer here... and it crashed when i did that...
    // not sure why, it was stack-allocated...
    if (str[size - 1] != 0x0) { // jic
        Serial.print("STRING WAS TOO BIG!: ");
        for (size_t i = 0; i < size; i++){
            Serial.print(str[i]);
        }
        Serial.println("");
    }

    data.push_back(packet_stamp::string);

    // this was also crashing for similar reasons...
    // trying to do an insert across the whole string...
    uint16_t size_small = (uint16_t)size;
    data.insert(data.end(), (uint8_t*)&size_small, (uint8_t*)(&size_small + 1));
    for (size_t i = 0; i < size; i++) {
        data.push_back((uint8_t)str[i]);
    }
}

void packet::pushTableBegin() { data.push_back(packet_stamp::table_begin); }
void packet::pushTableKey() { data.push_back(packet_stamp::table_key); }
void packet::pushTableValue() { data.push_back(packet_stamp::table_value); }
void packet::pushTableEnd() { data.push_back(packet_stamp::table_end); }
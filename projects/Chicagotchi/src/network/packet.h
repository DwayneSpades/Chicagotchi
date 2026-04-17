#ifndef _PACKET_H
#define _PACKET_H
#pragma once

#include <lua.h>
#include <esp_now.h>
#include <vector>
#include <cstdlib>

namespace packet_stamp {
    // primitives
    const uint8_t nil = 0x00;
    const uint8_t number = 0x01;
    const uint8_t boolean = 0x02;
    const uint8_t string = 0x03;

    // tables
    const uint8_t table_begin = 0x04;
    const uint8_t table_key = 0x05;
    const uint8_t table_value = 0x06;
    const uint8_t table_end = 0x07;
}

#define MAX_STR_SIZE 0xFF
struct packet_str_data {
    char buffer[MAX_STR_SIZE];
    uint16_t size = 0;
};

// what
size_t my_strlen_s(const char* buf, size_t max);

struct packet {
    uint8_t sender_addr[ESP_NOW_ETH_ALEN];
    std::vector<uint8_t> data;

    packet() {}
    
    packet(const uint8_t* src_addr);
    packet(const uint8_t* src_addr, const uint8_t* buf, int len);

    ~packet() {
        data.clear();
    }

    #define MAX_TABLE_DEPTH 10
    bool serialize(uint16_t packetId, lua_State* L, int idx);
    bool deserialize(lua_State* L);

private:
    bool processLuaTable(lua_State* L, int idx, int depth);
    bool pushLuaValue(lua_State* L, int idx, bool isKey);

    void pushPacketId(uint16_t id);
    void pushNil();
    void pushNumber(double value);
    void pushBoolean(bool value);
    void pushString(const char* str);
    void pushTableBegin();
    void pushTableKey();
    void pushTableValue();
    void pushTableEnd();
};

#endif
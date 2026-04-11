#ifndef _NETWORKMANAGER_H
#define _NETWORKMANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <lapi.h>
#include <vector>
#include <cstdint>
#include <string>

#include "luaState.h"

// ESP-NOW broadcast address
const uint8_t broadcastAddressFF[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool netInit = false;
bool peerInit = false;

// a fun little flag to check if communications are working
// will remove later
bool pinged = true;

// peer info structs
esp_now_peer_info_t peerInfo;

// this one is just for broadcast address
esp_now_peer_info_t peerInfoFF;

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

#define MAX_STR_SIZE 0xFFFF
struct packet_str_data {
    char buffer[MAX_STR_SIZE];
    uint16_t size = 0;
};

// what
size_t my_strlen_s(const char* buf, size_t max) {
    for (size_t i = 0; i < max; i++){
        if (*buf == 0x0) {
            return i+1;
        }

        buf++;
    }

    return max;
}

struct packet {
    std::vector<uint8_t> data;

    static bool deserialize(lua_State* L, const uint8_t* buf, int len) {
        lua_newtable(L);

        bool isKey = true; // false = isValue
        int tableCount = 0;

        for (int i = 0; i < len; i++) {
            switch(buf[i]) {
                // Primitives
                case packet_stamp::nil:
                    if (isKey) {
                        Serial.println("Abort! nils cannot be keys.");
                        return false;
                    } else {
                        lua_pushnil(L);
                    }
                    break;
                case packet_stamp::boolean:
                    lua_pushboolean(L, (int)buf[i]);
                    break;
                case packet_stamp::number:
                    // todo: copy eight bytes
                    lua_pushnumber(L, (double)buf[i]);
                    break;
                case packet_stamp::string:
                    lua_pushstring(L, (const char*)(buf+i));
                    break;

                // Tables
                case packet_stamp::table_begin:
                    if (isKey) {
                        Serial.println("Abort! Tables cannot be keys.");
                        return false;
                    } else {
                        tableCount++;
                        lua_newtable(L);
                    }
                    break;
                case packet_stamp::table_end:
                    tableCount--;
                    if (tableCount > 0) {
                        lua_settable(L, -3);
                    }
                    break;

                // Key/Value
                case packet_stamp::table_key:
                    // isKey starts true
                    // if its false, we need to pop a key/value pair
                    if (!isKey) {
                        lua_settable(L, -3);
                    }
                    isKey = true;
                    break;
                    
                case packet_stamp::table_value:
                    isKey = false;
                    break;
            }
        }

        if (tableCount > 0) {
            Serial.print("Abort! somehow ended up with a tableCount > 0: ");
            Serial.print(tableCount);
            Serial.println("");
            return false;
        }
        return true;
    }

    void pushNil() {
        data.push_back(packet_stamp::nil);
    }

    void pushNumber(double value) {
        data.push_back(packet_stamp::number);
        data.insert(data.end(), (uint8_t*)&value, (uint8_t*)(&value + 1));
    }

    void pushBoolean(bool value) {
        data.push_back(packet_stamp::boolean);
        data.push_back((uint8_t)value);
    }

    void pushString(const char* str) {
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

    void pushTableBegin() { data.push_back(packet_stamp::table_begin); }
    void pushTableKey() { data.push_back(packet_stamp::table_key); }
    void pushTableValue() { data.push_back(packet_stamp::table_value); }
    void pushTableEnd() { data.push_back(packet_stamp::table_end); }
};

// timing
const float SPASS_PERIOD_MS = 3000.0f;
float spass_time = 0.0f;

bool hasStreetPass = false;
// special message that just says "hey I want to connect!"
// todo: refine the packet system
uint8_t discovery_message[64] = {
    0x10,
    0x11,
    0x12,
    0x13,
    0x20,
    0x21,
    0x22,
    0x23,
};

bool isDiscoveryMessage(const uint8_t* msg, int len) {
    if (len < 8) return false;

    for (int i = 0; i < 8; i++) {
        if (msg[i] != discovery_message[i]) {
            return false;
        }
    }

    return true;
}

// utility function to send a mac address to the serial monitor
// there is likely an easier way lol
void SerialPrintMAC(const uint8_t* mac, const char* end = "") {
    Serial.print("[");
    for (int i = 0; i < 6; i++) {
        Serial.print("0x");
        Serial.print(*(mac + i), HEX);

        if (i < 5) {
            Serial.print(", ");
        }
    }
    Serial.print("]");
    Serial.print(end);
}

// adds a peer
// todo: make it add more than one
void addPeer(const esp_now_recv_info_t* esp_now_info) {
    peerInit = true;
    hasStreetPass = true;

    memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
    peerInfo.channel = 6;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add new peer");
        return;
    }
}

// function to manually send an arbitrary message
bool sendMsg(const uint8_t* peer_addr, const char* msg, int len) {
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(peer_addr, (const uint8_t*)msg, len);

    if (result == ESP_OK) {
        Serial.print("Sent '");
        Serial.print(msg);
        Serial.print("' to ");
        SerialPrintMAC(peer_addr);
        Serial.print(" with success.");
        return true;
    } else {
        Serial.print("Error sending the data: ");
        Serial.print(esp_err_to_name(result));
        Serial.print(". ");
        return false;
    }
}

// sends the discovery message
void broadcastDiscoveryMessage() {
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddressFF, discovery_message, sizeof(discovery_message));
    
    if (result == ESP_OK) {
        Serial.print("Sent broadcast with success.\n");
    }
    else {
        Serial.print("Error broadcasting: ");
        Serial.print(esp_err_to_name(result));
        Serial.print(". ");
    }
}

// callback when data is sent
void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status:\t");
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    Serial.print(": ");

    SerialPrintMAC(mac_addr->des_addr, "\n");
}


// callback for when data is received
void OnDataRecv(const esp_now_recv_info_t* esp_now_info, const uint8_t *data, int data_len) {
    Serial.println("Recv data: ");
    Serial.println((const char*)data);
    Serial.println("-");

    if (isDiscoveryMessage(data, data_len)) {
        Serial.println("Peer found: ");
        SerialPrintMAC(esp_now_info->src_addr, "\n");
        
        if (!peerInit) {
            addPeer(esp_now_info);
        }
    } else {
        lua_getglobal(L, "myrtle_on_packetrecv");
        if (lua_isfunction(L, -1))
        {
            if (packet::deserialize(L, data, data_len)) {
                lua_pcall(L, 0, 0, 0);
            } else {
                Serial.println("Error when deserializing...");
            }
        }

        Serial.println("pinged!");
        pinged = true;
    }
}

// call this to initialize the network
void networkSetup() {
    Serial.println("network setup :)");
    Serial.println("###################");

    // Set device as a Wi-Fi Station
    Serial.println("pre wifi mode");
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.println("wifi mode ok");
    delay(1000);
    WiFi.setChannel(6);
    Serial.println("wifi channel ok");
    delay(1000);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    Serial.println("ESP-NOW Init Ok!");

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Transmitted packet
    esp_err_t result = esp_now_register_send_cb(OnDataSent);
    if (result != ESP_OK) {
        Serial.print("Error registering a send cb: ");
        Serial.print(esp_err_to_name(result));
        Serial.println(". ");
    }
    result = esp_now_register_recv_cb(OnDataRecv);
    if (result != ESP_OK) {
        Serial.print("Error registering a recv cb: ");
        Serial.print(esp_err_to_name(result));
        Serial.println(". ");
    }
    
    // Register peer
    memcpy(peerInfoFF.peer_addr, broadcastAddressFF, 6);
    peerInfoFF.channel = 6;  
    peerInfoFF.encrypt = false;

    // not sure what this did...
    // peerInfoFF.ifidx = wifi_interface_t::WIFI_IF_AP;
    
    // Add broadcast peer        
    if (esp_now_add_peer(&peerInfoFF) != ESP_OK){
        Serial.println("Failed to add peer FF");
        return;
    }

    Serial.println("Added Peers Ok!");

    Serial.println("###################");
    Serial.println("everything ship-shape capn'!");
    Serial.println("###################");
    netInit = true;
}

// call this to let the network manager tick
void networkUpdate(float dt) {
    if (!netInit) return;

    // broadcast loop
    spass_time += dt;
    if (spass_time >= SPASS_PERIOD_MS) {
        spass_time -= SPASS_PERIOD_MS;
        broadcastDiscoveryMessage();
        return;
    }
}

// call after checking this flag
void networkClear() {
    if (!netInit) return;

    pinged = false;
}

// sends a message
bool networkSendPing() {
    if (!netInit) return false;

    if (peerInit) {
        const char msg[64] = "Nice to meet you :)";
        return sendMsg(peerInfo.peer_addr, msg, sizeof(msg));
    }

    return false;
}

bool consumeStreetpass() {
    if (hasStreetPass) {
        hasStreetPass = false;
        return true;
    }
    return false;
}

#define DBG_SER 1
#if DBG_SER

void printTypeAndValue(lua_State* L, int idx) {
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
        default:
            Serial.print("Err: Unsupported type '");
            Serial.print(tName);
            Serial.print("'");
        break;
    }
    Serial.println("");
}

int tab = 0;
void indent() {
    for (int i=0; i<tab; i++) {
        Serial.print("\t");
    }
}

void printLuaStack(lua_State* L) {
    for (int i = 1; i <= lua_gettop(L); i++) {
        Serial.print(i);
        Serial.print(": ");
        printTypeAndValue(L, i);
    }
}
#endif

bool pushLuaValue(packet& pck, lua_State* L, int idx, bool isKey) {
    if (isKey) {
        pck.pushTableKey();
    } else {
        pck.pushTableValue();
    }

#if DBG_SER
    printTypeAndValue(L, idx);
#endif

    int valueT = lua_type(L, idx);
    switch (valueT) {
        case LUA_TNIL:
            pck.pushNil();
            break;
        case LUA_TTABLE:
            // skip
            if (isKey) {
                Serial.println("Err: Tables as keys are not supported.'");
                return false;
            }
            break;
        case LUA_TNUMBER:
            pck.pushNumber(lua_tonumber(L, idx));
            break;
        case LUA_TBOOLEAN:
            pck.pushNumber(lua_toboolean(L, idx));
            break;
        case LUA_TSTRING:
            pck.pushString(lua_tostring(L, idx)); // <- this is cursed -- might be brittle...
            break;
        default:
            pck.pushNil();
            Serial.print("Err: Unsupported type '");
            Serial.print(lua_typename(L, valueT));
            Serial.println("'");
            return false;
        break;
    }

    return true;
}

// todo: max depth check -- don't want to stack overflow
void processLuaTable(packet& pck, lua_State* L, int idx) {
    pck.pushTableBegin();

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
        if (pushLuaValue(pck, L, -2, true)) { // key
            pushLuaValue(pck, L, -1, false); // value

            int valueT = lua_type(L, -1);
            if (valueT == LUA_TTABLE) {
#if DBG_SER
                indent();
                Serial.println("{");
                tab++;
#endif

                processLuaTable(pck, L, lua_gettop(L));
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

    pck.pushTableEnd();
}

int lua_testPacket(lua_State* L) {
    packet pck;
    int t = lua_type(L, 1);

#if DBG_SER
    Serial.println("packet::serialize - ");
    Serial.println("Pre next == ");
    printLuaStack(L);
#endif
    if (t == LUA_TTABLE) {
        processLuaTable(pck, L, 1);
    } else {
        Serial.print("packet::serialize: unsupported parameter type '");
        Serial.print(lua_typename(L, t));
        Serial.println("'.");
    }

#if DBG_SER
    Serial.println("Post next == ");
    printLuaStack(L);

    Serial.println("PACKET: ");
    for (size_t i = 0; i < pck.data.size(); i++) {
        Serial.print("0x");
        Serial.print(pck.data[i], HEX);
        Serial.print(", ");
    }
    Serial.println(" | ");
#endif
    lua_pop(L, 1);

    if (peerInit) {
        sendMsg(peerInfo.peer_addr, (const char*)pck.data.data(), pck.data.size());
    }
    return 1;
}

#endif
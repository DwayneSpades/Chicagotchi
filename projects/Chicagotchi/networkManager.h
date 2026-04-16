#ifndef _NETWORKMANAGER_H
#define _NETWORKMANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <lapi.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
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

std::mutex msg_mutex;

#define DBG_USE_LOOPBACK 0
#define DBG_SER 0

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

#if DBG_SER
int tab = 0;
void indent() {
    for (int i=0; i<tab; i++) {
        Serial.print("\t");
    }
}
#endif

void printLuaStack(lua_State* L) {
    if (lua_gettop(L) < 1) {
        Serial.println("\t\t0: printLuaStack: lua_gettop <= 0");
    } else {
        for (int i = 1; i <= lua_gettop(L); i++) {
            Serial.print("\t\t");
            Serial.print(i);
            Serial.print(": ");
            printTypeAndValue(L, i);
        }
        Serial.println("-");
    }
}

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
    uint8_t sender_addr[ESP_NOW_ETH_ALEN];
    std::vector<uint8_t> data;

    packet() {}
    
    packet(const uint8_t* src_addr) {
        memcpy(sender_addr, src_addr, ESP_NOW_ETH_ALEN);
    }

    packet(const uint8_t* src_addr, const uint8_t* buf, int len) {
        memcpy(sender_addr, src_addr, ESP_NOW_ETH_ALEN);
        data.insert(data.end(), buf, buf+len);
    }

    ~packet() {
        data.clear();
    }

    bool deserialize(lua_State* L) {
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
            printTypeAndValue(L, 1);
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

    void pushPacketId(uint16_t id) {
        if (data.size() > 0) {
            Serial.print("Err -- packetId should be the first item! Attempted to push id: ");
            Serial.print(id);
            Serial.println("");
            return;
        }

        data.insert(data.end(), (uint8_t*)&id, (uint8_t*)(&id + 1));
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

#define PING_STRIKES 3
struct pingInfo {
    int strikes = -1;
};

#define MAX_PACKETS 10
std::vector<packet> packets;

struct addrHasher {
    std::size_t operator()(const std::array<uint8_t, 6>& arr) const {
        std::size_t hash = 17;

        for (int i = 0; i < 6; i++)
        {
            hash = hash * 31 + static_cast<std::size_t>(arr[i]);
        }

        return hash;
    }
};

std::unordered_map<std::array<uint8_t, 6>, pingInfo, addrHasher> peerPingInfo;

// timing
const float DISCOVERY_PERIOD_MS = 5000.0f;
float discovery_time = 0.0f;

const float PING_PERIOD_MS = 5000.0f;
float ping_time = 0.0f;

// special message that just says "hey I want to connect!"
// todo: replace this lol
uint8_t discovery_message[64] = {
    0x10,
    0x11,
    0x12,
    0x13,
    0x20,
    0x21,
    0x22,
    0x00,
};

// special message that just says "are ya still there"
// todo: replace this lol
uint8_t ping_message[64] = {
    0x04,
    0x04,
    0x04,
    0x04,
    0x01,
    0x01,
    0x01,
    0x00,
};

// special message that just says "im still here"
// todo: replace this lol
uint8_t ping_ack_message[64] = {
    0x09,
    0x09,
    0x09,
    0x09,
    0x06,
    0x06,
    0x06,
    0x00,
};

bool isMessage(const uint8_t* msg1, const uint8_t* msg2, int len) {
    if (len < 8) return false;

    for (int i = 0; i < 8; i++) {
        if (msg1[i] != msg2[i]) {
            return false;
        }
    }

    return true;
}

bool isDiscoveryMessage(const uint8_t* msg, int len) { return isMessage(msg, discovery_message, len); }
bool isPingMessage(const uint8_t* msg, int len) { return isMessage(msg, ping_message, len); }
bool isPingAckMessage(const uint8_t* msg, int len) { return isMessage(msg, ping_ack_message, len); }

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

void onDiscovery(const uint8_t* peer_addr) {
    lua_getglobal(L, "myrtle_on_peer_discovery");
    if (lua_isfunction(L, -1)) {
        lua_pushlstring(L, (const char*)peer_addr, 6);
        lua_pcall_custom(L, 1, 0, 0);
    } else {
        Serial.println("Discovery function not found");
    }
}

void addPeer(const esp_now_recv_info_t* esp_now_info) {
    peerInit = true;

    memcpy(peerInfo.peer_addr, esp_now_info->src_addr, ESP_NOW_ETH_ALEN);
    peerInfo.channel = 6;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add new peer");
        return;
    }

    std::array<uint8_t, 6> arr;
    std::copy(peerInfo.peer_addr, peerInfo.peer_addr + 6, arr.begin());

    peerPingInfo[arr] = pingInfo();

    onDiscovery(peerInfo.peer_addr);
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
    std::lock_guard<std::mutex> lock(msg_mutex);

    Serial.print("Last Packet Send Status:\t");
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    Serial.print(": ");

    SerialPrintMAC(mac_addr->des_addr, "\n");
}


// callback for when data is received
void OnDataRecv(const esp_now_recv_info_t* esp_now_info, const uint8_t *data, int data_len) {
    std::lock_guard<std::mutex> lock(msg_mutex);

    Serial.println("Recv data: ");
    Serial.println((const char*)data);
    Serial.println("-");

    if (isDiscoveryMessage(data, data_len)) {
        Serial.println("Peer found: ");
        SerialPrintMAC(esp_now_info->src_addr, "\n");
        
        if (!esp_now_is_peer_exist(esp_now_info->src_addr)) {
            addPeer(esp_now_info);
        }
    } else if (isPingMessage(data, data_len)) {
        sendMsg(esp_now_info->src_addr, (const char*)ping_ack_message, 8);
    } else if (isPingAckMessage(data, data_len)) {
        std::array<uint8_t, 6> arr;
        std::copy(esp_now_info->src_addr, esp_now_info->src_addr + 6, arr.begin());
        
        if (peerPingInfo.contains(arr)){
            peerPingInfo[arr].strikes = -1;
        }
    } else {
#if DBG_SER
        Serial.print("Pushing a packet: ");
        for (int i = 0; i < std::min(4, data_len); i++){
            Serial.print("0x");
            Serial.print(data[i], HEX);
            Serial.print(", ");
        }
        Serial.println("");
#endif
        if (packets.size() < MAX_PACKETS) {
            packets.push_back(packet(esp_now_info->src_addr, data, data_len));
        } else {
            Serial.println("Error: dropping this packet -- i got too many.");
        }
    }
}

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

#define MAX_TABLE_DEPTH 10
bool processLuaTable(packet& pck, lua_State* L, int idx, int depth) {
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

                if (depth > MAX_TABLE_DEPTH) {
                    Serial.print("Abort! exceeded max table depth: ");
                    Serial.print(MAX_TABLE_DEPTH);
                    Serial.println("");
                    return false;
                }
                if (!processLuaTable(pck, L, lua_gettop(L), depth++)) {
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

    pck.pushTableEnd();
    return true;
}

// todo: make a macro
bool espNowCall(esp_err_t code, const char* func) {
    if (code == ESP_OK) {
        return true;
    }

    Serial.print("Err: ");
    Serial.print(func);
    Serial.print(" returned '");
    Serial.print(esp_err_to_name(code));
    Serial.println("'");
    return false;
}

int lua_sendMessage(lua_State* L) {
#if DBG_SER
    Serial.println("SEND MESSAGE =========================================================");
#endif
    String macStr = WiFi.STA.macAddress();
    packet pck((const uint8_t*)macStr.c_str());

    if (lua_type(L, 1) != LUA_TSTRING) {
        Serial.println("sendMessage Error -- arg 1 should be a string");
        return 1;
    }

    const uint8_t* destAddr = (const uint8_t*)lua_tostring(L, 1);

    // maybe enforce a bounds check here
    uint16_t packetId = static_cast<uint16_t>(lua_tointeger(L, 2));

    bool success = false;
    int t = lua_type(L, 3);
#if DBG_SER
    Serial.println("packet::serialize - ");
    Serial.println("Pre next == ");
    printLuaStack(L);
#endif
    if (t == LUA_TTABLE) {
        pck.pushPacketId(packetId);
        success = processLuaTable(pck, L, 3, 0);
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
        if (success) {
            sendMsg(destAddr, (const char*)pck.data.data(), pck.data.size());
        } else {
            Serial.print("Failed to send packet with id ");
            Serial.print(packetId);
            Serial.println("");
        }
    }

#if DBG_USE_LOOPBACK
    packets.push_back(packet(destAddr, pck.data.data(), pck.data.size()));
#endif

    return 1;
}

int getDirectPeerCount() {
    esp_now_peer_num_t num;
    if (espNowCall(esp_now_get_peer_num(&num), "esp_now_get_peer_num")) {
        return num.total_num - 1;
    } else {
        return 0;
    }
}

int lua_getPeerCount(lua_State* L) {
#if DBG_SER
    Serial.println("GET PEER COUNT =========================================================");
#endif
    lua_pushnumber(L, (double)getDirectPeerCount());

    return 1;
}

int lua_getPeerAddr(lua_State* L) {
#if DBG_SER
    Serial.println("GET PEER ADDR =========================================================");
#endif
    int index = lua_tointeger(L, 1); // assumes we're getting a lua-index (first=1)

    bool success = false;
    esp_now_peer_info_t peer;
    for (int i = 0; i < index; i++) {
        bool result = espNowCall(esp_now_fetch_peer(i == 0, &peer), "esp_now_fetch_peer");
        if (i == index - 1) {
            success = result;
        }
    }

    if (!success) {
        memset(peer.peer_addr, 'X', ESP_NOW_ETH_ALEN);
    }
    lua_pushlstring(L, (const char*)peer.peer_addr, 6);

    return 1;
}

// call this to initialize the network
void networkSetup() {
    Serial.println("network setup :)");
    Serial.println("###################");

    // Set device as a Wi-Fi Station
    Serial.println("pre wifi mode");
    delay(100);
    WiFi.mode(WIFI_STA);
    Serial.println("wifi mode ok");
    delay(100);
    WiFi.setChannel(6);
    // 80db!!!
    WiFi.setTxPower(wifi_power_t::WIFI_POWER_2dBm);
    Serial.print("txPower: ");
    Serial.print(WiFi.getTxPower());
    Serial.println("dB");
    Serial.println("wifi channel ok");
    delay(100);

    espNowCall(esp_wifi_set_ps(wifi_ps_type_t::WIFI_PS_MAX_MODEM), "esp_wifi_set_ps");

    wifi_ps_type_t ps_type;
    if (espNowCall(esp_wifi_get_ps(&ps_type), "esp_wifi_get_ps")) {
        Serial.print("Using PS Mode: ");
        switch (ps_type) {
            case wifi_ps_type_t::WIFI_PS_NONE:
                Serial.print("WIFI_PS_NONE");
            break;
            case wifi_ps_type_t::WIFI_PS_MIN_MODEM:
                Serial.print("WIFI_PS_MIN_MODEM");
            break;
            case wifi_ps_type_t::WIFI_PS_MAX_MODEM:
                Serial.print("WIFI_PS_MAX_MODEM");
            break;
            default:
                Serial.print("Unkown wifi ps type");
            break;
        }
        Serial.println("");
    }

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
    std::lock_guard<std::mutex> lock(msg_mutex);

    // broadcast loop
    discovery_time += dt;
    if (discovery_time >= DISCOVERY_PERIOD_MS) {
        Serial.println("Broadcasting Discovery Message");
        discovery_time -= DISCOVERY_PERIOD_MS;
        broadcastDiscoveryMessage();
        return;
    }
    
    // ping loop
    ping_time += dt;
    if (ping_time >= PING_PERIOD_MS) {
        ping_time -= PING_PERIOD_MS;

        if (peerPingInfo.size() > 0) {
            Serial.println("Broadcasting Ping Message");
        } else {
            Serial.println("No peers to ping");
            return;
        }

        for (auto it = peerPingInfo.begin(); it != peerPingInfo.end();) {
            if (++it->second.strikes > PING_STRIKES) {
                uint8_t addr[ESP_NOW_ETH_ALEN];
                memcpy(addr, it->first.data(), sizeof(uint8_t) * ESP_NOW_ETH_ALEN);

                esp_now_del_peer(it->first.data());
                it = peerPingInfo.erase(it);

                Serial.print("Update: Lost a peer!");
                SerialPrintMAC(addr, "\n");

                lua_getglobal(L, "myrtle_on_peer_lost");
                if (lua_isfunction(L, -1)) {
                    lua_pushlstring(L, (const char*)addr, 6);
                    lua_pcall_custom(L, 1, 0, 0);
                } else {
                    Serial.println("Peer Lost function not found");
                }

            } else {
                ++it;
            }
        }

        bool success = false;
        esp_now_peer_info_t peer;
        int peerCount = getDirectPeerCount();
        // it seems to count the broadcast address in the peer count
        // but not in the peer list...
        for (int i = 0; i < peerCount; i++) {
            if (espNowCall(esp_now_fetch_peer(i == 0, &peer), "esp_now_fetch_peer")) {
                sendMsg(peer.peer_addr, (const char*)ping_message, 8);
            }
        }
        return;
    }

    for (size_t i = 0; i < packets.size(); i++) {
#if DBG_SER
        Serial.print("packet(");
        Serial.print(i);
        Serial.print(") ");
#endif
        lua_getglobal(L, "myrtle_on_packetrecv");
        if (lua_isfunction(L, -1)) {
#if DBG_SER
            Serial.println("pre deser: ");
            printLuaStack(L);
#endif
            if (packets[i].deserialize(L)) {
#if DBG_SER
                Serial.println("calling: ");
                printLuaStack(L);
#endif

                lua_pcall_custom(L, 3, 0, 0);
            } else {
                lua_settop(L, 0);
                Serial.println("Error when deserializing...");
            }
        }

#if DBG_SER
        Serial.println("post deser: ");
        printLuaStack(L);
#endif
    }

    packets.clear();
}

// call after checking this flag
void networkClear() {
    if (!netInit) return;

    pinged = false;
}

#endif
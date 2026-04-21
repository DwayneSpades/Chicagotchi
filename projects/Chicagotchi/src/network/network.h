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

#include "..\luaState.h"
#include "..\luaUtil.h"
#include "packet.h"

namespace network {
    // ESP-NOW broadcast address
    const uint8_t broadcastAddressFF[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    bool netInit = false;
    bool peerInit = false;

    // peer info structs
    esp_now_peer_info_t peerInfo;

    // this one is just for broadcast address
    esp_now_peer_info_t peerInfoFF;

    std::mutex msg_mutex;

    #define DBG_USE_LOOPBACK 0
    #define DBG_SER 0

    #if DBG_SER
    int tab = 0;
    void indent() {
        for (int i=0; i<tab; i++) {
            Serial.print("\t");
        }
    }
    #endif

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
        luaUtil::printLuaStack(L);
    #endif
        if (t == LUA_TTABLE) {
            success = pck.serialize(packetId, L, 3);
        } else {
            Serial.print("packet::serialize: unsupported parameter type '");
            Serial.print(lua_typename(L, t));
            Serial.println("'.");
        }

    #if DBG_SER
        Serial.println("Post next == ");
        luaUtil::printLuaStack(L);

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
#if DBG_USE_LOOPBACK
        lua_pushnumber(L, 1.0);
#else
        lua_pushnumber(L, (double)getDirectPeerCount());
#endif

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
    void setup() {
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
    void update(float dt) {
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
                luaUtil::printLuaStack(L);
    #endif
                if (packets[i].deserialize(L)) {
    #if DBG_SER
                    Serial.println("calling: ");
                    luaUtil::printLuaStack(L);
    #endif

                    lua_pcall_custom(L, 3, 0, 0);
                } else {
                    lua_settop(L, 0);
                    Serial.println("Error when deserializing...");
                }
            }

    #if DBG_SER
            Serial.println("post deser: ");
            luaUtil::printLuaStack(L);
    #endif
        }

        packets.clear();
    }
}

#endif
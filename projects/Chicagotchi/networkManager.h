#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

const uint8_t broadcastAddressFF[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool peerInit = false;

esp_now_peer_info_t peerInfo;
esp_now_peer_info_t peerInfoFF;

const float SPASS_PERIOD_MS = 3000.0f;
float spass_time = 0.0f;

// do i need this still
uint8_t peer_message[64] = {
    0x10,
    0x11,
    0x12,
    0x13,
    0x20,
    0x21,
    0x22,
    0x23,
};

bool isPeerMessage(const uint8_t* msg, int len) {
    if (len < 8) return false;

    for (int i = 0; i < 8; i++) {
        if (msg[i] != peer_message[i]) {
            return false;
        }
    }

    return true;
}

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

void addPeer(const esp_now_recv_info_t* esp_now_info) {
    peerInit = true;
    memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
    peerInfo.channel = 6;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add new peer");
        return;
    }
}

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
    }
    else {
        Serial.print("Error sending the data: ");
        Serial.print(esp_err_to_name(result));
        Serial.print(". ");
        return false;
    }
}

void sendBroadcast(const uint8_t* peer_addr) {
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(peer_addr, peer_message, sizeof(peer_message));
    
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
    // this was buzz

    Serial.print("Last Packet Send Status:\t");
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    Serial.print(": ");

    SerialPrintMAC(mac_addr->des_addr, "\n");
}

void OnDataRecv(const esp_now_recv_info_t* esp_now_info, const uint8_t *data, int data_len) {
    // this was buzz

    Serial.println("Recv data: ");
    Serial.println((const char*)data);
    Serial.println("-");

    if (isPeerMessage(data, data_len)) {
        Serial.println("Peer found: ");
        SerialPrintMAC(esp_now_info->src_addr, "\n");
        
        if (!peerInit) {
            addPeer(esp_now_info);
        }
    }
    
    if (peerInit) {
        const char msg[64] = "Nice to meet you :)";
        sendMsg(peerInfo.peer_addr, msg, sizeof(msg));
    }
}

void networkSetup() {
    Serial.println("network setup :)");
    Serial.println("###################");

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    Serial.println("wifi mode ok");
    WiFi.setChannel(6);
    Serial.println("wifi channel ok");

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    Serial.println("ESP-NOW Init Ok!");

    /*
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
    */
}

void networkUpdate(float dt) {
    // broadcast loop
    spass_time += dt;
    if (spass_time >= SPASS_PERIOD_MS) {
        spass_time -= SPASS_PERIOD_MS;
        sendBroadcast(broadcastAddressFF);
        return;
    }
}

bool networkSendPing() {
    if (peerInit) {
        const char msg[64] = "Nice to meet you :)";
        return sendMsg(peerInfo.peer_addr, msg, sizeof(msg));
    }

    return false;
}

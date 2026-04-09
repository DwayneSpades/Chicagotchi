#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ESP-NOW broadcast address
const uint8_t broadcastAddressFF[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool peerInit = false;

// a fun little flag to check if communications are working
// will remove later
bool pinged = true;

// peer info structs
esp_now_peer_info_t peerInfo;

// this one is just for broadcast address
esp_now_peer_info_t peerInfoFF;

// timing
const float SPASS_PERIOD_MS = 3000.0f;
float spass_time = 0.0f;

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
    }
    else {
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
        Serial.println("pinged!");
        pinged = true;
    }
}

// call this to initialize the network
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
}

// call this to let the network manager tick
void networkUpdate(float dt) {
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
    pinged = false;
}

// sends a message
bool networkSendPing() {
    if (peerInit) {
        const char msg[64] = "Nice to meet you :)";
        return sendMsg(peerInfo.peer_addr, msg, sizeof(msg));
    }

    return false;
}

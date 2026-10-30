// SPDX-License-Identifier: MIT
/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  ESP32+HX711 - Load Cell Weight Sensor
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  Features:
 *  - HTTP GET requests to configurable URLs
 *  - WiFi connection with auto-reconnect
 *  - Serial communication for weight reports
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>

#include "src/constants.h"
#include "src/smart_scale.h"

SmartScale* SCALE;

void setup() {
    Serial.begin(9600);
    delay(500); // Wait for serial..

    // read MAC address to create 4 bytes ID
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    char mac_id[4];
    sprintf(mac_id, "%02x%02x", mac[4], mac[5]);

    // e.g. with PREFIX="sensor.weight_", name="sensor.weight_aabb".
    String name = String(String(PREFIX)+String(mac_id));

    SCALE = new SmartScale(name.c_str(), ESP32HX711_VERSION);
    SCALE->SetSensor("weight", LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    SCALE->Setup();

    delay(50);
    Serial.println();
    Serial.printf("[OK] Firmware version: %s\n", ESP32HX711_VERSION);
}

void loop() {
    SCALE->OnLoop();
}

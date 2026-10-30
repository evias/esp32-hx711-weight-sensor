// SPDX-License-Identifier: MIT
#ifndef ESP32HX711_CONSTANTS_H
#define ESP32HX711_CONSTANTS_H

#define ESP32HX711_VERSION "0.1.0-alpha.1"

/**
 * Set a different name prefix in .vscode/arduino.json.
 * e.g.: `-DPREFIX="sensor.my_sensor_name_"`
 *
 * Please account for the fact that the 4 ending bytes of the device's
 * MAC address will always be added at the end of the name.
 */
#ifndef PREFIX
    #define PREFIX "sensor.weight_sensor_"
#endif

/**
 * Don't forget to change the WiFi SSID in .vscode/arduino.json.
 * e.g.: `-DWIFI_SSID="YourWiFiNetwork"`
 */
#ifndef WIFI_SSID
    #define WIFI_SSID "Default"
#endif

/**
 * Don't forget to change the WiFi password in .vscode/arduino.json.
 * e.g.: `-DWIFI_PASS="YourWiFiPassword"`
 */
#ifndef WIFI_PASS
    #define WIFI_PASS "Default"
#endif

/**
 * You may enable debug mode in .vscode/arduino.json.
 * e.g.: `-DDEBUG_MODE=1`
 */
#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
#endif

/**
 * You may enable quiet mode in .vscode/arduino.json.
 * e.g.: `-DQUIET_MODE=1`
 */
#ifndef QUIET_MODE
    #define QUIET_MODE 0
#endif

/**
 * Don't forget to change the Webhook URL in .vscode/arduino.json.
 * e.g.: `-DWEBHOOK_URL="http://homeassistant.local:8123/api/webhook/sensors"`
 */
#ifndef WEBHOOK_URL
    #define WEBHOOK_URL "http://localhost:8123/api/webhook/sensors"
#endif

/**
 * You may change the number of batch readings in .vscode/arduino.json.
 * e.g.: `-DBATCH_SIZE=10`
 */
#ifndef BATCH_SIZE
    #define BATCH_SIZE 10
#endif

/**
 * You may change the time interval (in millisecond) between readings in .vscode/arduino.json.
 * e.g.: `-DINTERVAL=500`
 */
#ifndef INTERVAL
    #define INTERVAL 500
#endif

constexpr bool ESP32HX711_DEBUG_ENABLED = (bool) DEBUG_MODE;
constexpr bool ESP32HX711_QUIET_ENABLED = (bool) DEBUG_MODE;
constexpr char ESP32HX711_WIFI_SSID[] = WIFI_SSID;
constexpr char ESP32HX711_WIFI_PASS[] = WIFI_PASS;
constexpr char URL_DIAGNOSTIC_REPORTS[] = WEBHOOK_URL;
constexpr uint8_t ESP32HX711_BATCH_SIZE = BATCH_SIZE; // uint8_t (max 255) to restrict max size of batches
constexpr uint32_t ESP32HX711_INTERVAL = INTERVAL;

/**
 * Timing Configuration (in milliseconds)
 */
constexpr unsigned long DEBOUNCE_DELAY = 50; // Always added to interval!
constexpr unsigned long HTTP_TIMEOUT = 5000;  // 5 seconds HTTP request timeout

/**
 * 2-pin Press Button
 *
 * esp32(D15) -> B(1)
 * esp32(GND) -> B(2)
 **/
const unsigned short LOADCELL_DOUT_PIN = 16;
const unsigned short LOADCELL_SCK_PIN  = 4;

#endif
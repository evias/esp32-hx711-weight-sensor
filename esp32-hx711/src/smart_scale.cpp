// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "smart_scale.h"
#include "constants.h"
#include "serial.h"

/// @brief SmartScale public constructor, says Hi.
/// @param name A friendly name for your SmartScale, e.g. "sensor.".
/// @param version A version number, recommended semantic versioning, e.g. "1.0.0".
SmartScale::SmartScale(const char* name, const char* version)
    : name_(String(name)),
      version_(String(version)),
      online_(false),
      ip_address_("Unknown"),
      mac_address_("Unknown"),
      has_module_(false),
      batch_({})
{
    webhook_enabled_  = strcmp(URL_DIAGNOSTIC_REPORTS, "") != 0;
    http_batch_size_  = ESP32HX711_BATCH_SIZE;
    is_quiet_mode_    = ESP32HX711_QUIET_ENABLED == 1;
    interval_between_ = ESP32HX711_INTERVAL + DEBOUNCE_DELAY;

    // batch size must be `0 < x <= 100`.
    if (http_batch_size_ <= 0 || http_batch_size_ > 100) {
        http_batch_size_ = 10; // fallback to default
    }

    sendMessage(String("Hola! I am Biscuit :]"), "HOLA");
}

/// @brief The Setup method initializes configured pin numbers.
/// @details This method is called from the sketch's setup() function.
///
/// CAUTION: If tare_offset is 0 (unset), the firmware enters in CALIBRATION
/// mode, the user must place a 100gram item on the scale. Calibration
/// waits 10 seconds then takes a raw reading to compute a factor.
void SmartScale::Setup()
{
    if (!has_module_) {
        sendErrorMessage(String("ERROR: HX711 not configured, missing call to SmartScale#SetSensor."));
        delay(1000);
        return ;
    }

    // Configure HX711 pin
    bool ready = false;
    do {
        hx711_module_.mod.begin(hx711_module_.dev.pins[0], hx711_module_.dev.pins[1]);
        hx711_module_.mod.set_gain(128); // Channel A, gain 128 (default for load cells)

        if (hx711_module_.mod.wait_ready_timeout(3000)) {
            sendDebugMessage(String("HX711 ready."));
            ready = true;
            break;
        }

        sendErrorMessage(String("ERROR: HX711 not found. Check wiring (DT/SCK swapped is common)."));
        sendDebugMessage(String("Retrying after 1s..."));
        delay(1000);
    } while(!ready);
    delay(50);

    // Read tare offset and calibration factor from Settings
    settings_.begin("hx711", true);  // true = read only
    long tare_offset = settings_.getLong("tare_offset", 0);
    float calibration_factor = settings_.getFloat("calibration_factor", 1.0f);
    settings_.end();

    // Enter CALIBRATION mode
    if (tare_offset == 0) {
        sendDebugMessage(String("Entering CALIBRATION mode."));

        hx711_module_.mod.tare(20); // average 20 readings as zero point
        tare_offset_ = hx711_module_.mod.get_offset();

        sendMessage(String("CAUTION: Place a weight of 100g on the scale."));
        sendMessage(String("Waiting 10s..."));
        delay(10000);

        // Execute 1 raw reading with 100g on it to compute calibration factor.
        sendMessage(String("CAUTION: Now extracting raw reading."));

        // Calibration factor is the raw reading divided by a known weight of 100g.
        // i.e. if raw=95, then factor is 95/100=0.95.
        long raw = hx711_module_.mod.read(); // raw 24-bit value
        calibration_factor_ = raw / 100;

        sendDebugMessage(String(
            "tare=" + String(tare_offset_) + " ; raw=" + String(raw) + " ; factor=" + String(calibration_factor_)
        ));

        settings_.begin("hx711", false);  // false = read/write
        settings_.putLong("tare_offset", tare_offset_);
        settings_.putFloat("calibration_factor", calibration_factor_);
        settings_.end();

        sendDebugMessage(String("Calibration done."));
    }
    // CALIBRATION already done, read from Settings
    else {
        tare_offset_ = tare_offset;
        calibration_factor_ = calibration_factor;
    }

    setupWiFiConnection();
    sendDebugMessage(String("SmartScale initialized"));
}

/// @brief The OnLoop method produces raw readings from the sensor.
/// @details This method is called from the sketch's loop() function.
void SmartScale::OnLoop()
{
    if (hx711_module_.mod.wait_ready_timeout(200)) {
        // Execute raw readings and compute weight in grams.
        long raw = hx711_module_.mod.read(); // raw 24-bit value
        float grams = (raw - tare_offset_) / calibration_factor_;

        // Appends SensorReading entry to current batch.
        SensorReading reading = {
            .read_at = millis(),
            .name    = hx711_module_.dev.id,
            .unit    = String("g"),
            .value   = String(grams)
        };
        batch_.push_back(reading);

        if (!is_quiet_mode_) {
            Serial.println(reading.ToString());
        }

        // We know that batch_ has a minimum size of 1.
        bool should_send_batch = batch_.size() % http_batch_size_ == 0;
        if (webhook_enabled_ && should_send_batch) {
            // Consume the batch of readings (and clear batch_).
            String data = consumeBatch();

            // Send over HTTP
            sendHTTPRequest(URL_DIAGNOSTIC_REPORTS, data.c_str());
        }

        // TODO(evias): SPIFFS storage in case of webhook_enabled=false
    }

    delay(interval_between_); // INTERVAL + DEBOUNCE_DELAY
}

/// @brief The SetSensor method registers a HX711 module's pins.
/// @param id A name for the component, e.g. "my-hx711-module-01".
/// @param dt The DOUT pin number (on the esp32) of this hx711 module.
/// @param sck The SCK pin number (on the esp32) of this hx711 module.
void SmartScale::SetSensor(const char* id, unsigned short dt, unsigned short sck)
{
    HX711 mod;
    hx711_module_ = WeightSensorDevice{
        mod,
        Device<2>{String(id), {dt, sck}}
    };
    has_module_ = true;
}

/// @brief The GetName method returns the SmartScale name.
/// @return The name of the SmartScale instance.
const String& SmartScale::GetName() {
    return this->name_;
}

/// @brief The GetVersion method returns the SmartScale version.
/// @return The version of the SmartScale instance.
const String& SmartScale::GetVersion() {
    return this->version_;
}

/// @brief The IsOnline method returns whether the SmartScale is connected to WiFi.
/// @return Returns true if the SmartScale is connected to a WiFi network.
bool SmartScale::IsOnline() {
    return this->online_;
}

/// @brief The GetIPAddress method returns the local IP address if available.
/// @return Returns a local IP address when connected to a WiFi network.
const String &SmartScale::GetIPAddress() {
    return this->ip_address_;
}

/// @brief The GetMACAddress method returns the MAC address if available.
/// @return Returns the MAC address of the WiFi adapter.
const String &SmartScale::GetMACAddress() {
    return this->mac_address_;
}

/// @brief The setupWiFiConnection method configures the WiFi if needed.
/// @details See also .vscode/arduino.json to update your credentials.
/// CAUTION: WIFI_SSID and WIFI_PASS are defined with -D build flags.
void SmartScale::setupWiFiConnection()
{
    // Read credentials if possible
    settings_.begin("wifi", true);  // true = read only
    String ssid = settings_.getString("ssid", "");
    String password = settings_.getString("password", "");
    settings_.end();

    // Save credentials if necessary
    // See .vscode/arduino.json to update WIFI_SSID, WIFI_PASS.
    bool has_wifi_ssid = 0 != strcmp(ESP32HX711_WIFI_SSID, "Default");
    bool has_wifi_pass = 0 != strcmp(ESP32HX711_WIFI_PASS, "Default");
    if (ssid.isEmpty() && has_wifi_ssid && has_wifi_pass) {
        settings_.begin("wifi", false);  // false = read/write
        settings_.putString("ssid", ESP32HX711_WIFI_SSID);
        settings_.putString("password", ESP32HX711_WIFI_PASS);
        ssid = String(ESP32HX711_WIFI_SSID);
        password = String(ESP32HX711_WIFI_PASS);
        settings_.end();

        sendDebugMessage(String("WiFi credentials saved"));
    }

    // Connect to WiFi if possible
    if (!ssid.isEmpty()) {
        if constexpr(ESP32HX711_DEBUG_ENABLED) {
            Serial.print(String("Connecting to WiFi")); // don't \n!
        }

        WiFi.begin(ssid.c_str(), password.c_str());

        bool hasError = false;
        bool isOnline = false;
        int  numAttempts = 0;
        while (!isOnline && !hasError && ++numAttempts < 10) {
            if constexpr(ESP32HX711_DEBUG_ENABLED) {
                Serial.print(String(".")); // don't \n!
            }

            switch (WiFi.status()) {
            case WL_CONNECTED:
                isOnline = true;
                Serial.println();
            break;

            case WL_CONNECT_FAILED:
            case WL_NO_SSID_AVAIL:
                hasError = true;
            break;
            default:
            break;
            }
            delay(500);
        }

        if (isOnline) {
            sendMessage(String("Connection to WiFi established!"));
            online_ = true;
            ip_address_ = WiFi.localIP().toString();
            mac_address_ = WiFi.macAddress();
        } else {
            sendMessage(String("\nCould not establish WiFi connection."), "WARN");
        }
    } else {
        sendMessage(String("WiFi is unable to connect: missing credentials."), "WARN");
    }
}

/// @brief The ensureWiFiConnected method ensures WiFi connectivity.
/// @details This method is called before sending HTTP requests.
void SmartScale::ensureWiFiConnected()
{
    if (WiFi.status() == WL_CONNECTED) {
        online_ = true;
        return;
    }

    sendMessage(String("WiFi disconnected, reconnecting..."), "WARN");
    online_ = false;

    WiFi.disconnect();
    setupWiFiConnection();
}

/// @brief The consumeBatch method formats a JSON array with readings.
/// @details CAUTION: Calling this method will clear the current batch.
const String& SmartScale::consumeBatch()
{
    String data("");
    for (int i = 0, m = batch_.size(); i < m; i++) {
        data += batch_[i].ToString();
        if (i + 1 < m) data += ",";
    }
    batch_.clear(); // BATCH is now empty

    return String("[") + data.c_str() + "]";
}

/// @brief The sendHTTPRequest method sends a HTTP request to a HASS webhook.
/// @details This method ensures WiFi connectivity or fails.
/// @param url The URL to which to send the HTTP request.
bool SmartScale::sendHTTPRequest(const char* url, const char* data)
{
    // Ensure WiFi is connected
    ensureWiFiConnected();

    if (!online_) {
        sendMessage(String("Cannot call webhook - WiFi is not connected"), "WARN");
        return false;
    }

    if (0 == batch_.size()) {
        sendMessage(String("Cannot call webhook - batch is empty"), "WARN");
        return false;
    }

    sendMessage(String("Sending HTTP POST request to:"), "INFO");
    sendMessage(String(url), "INFO");

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);

    // Add headers if needed
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device", "biscuit-smart-scale");

    String payload_json = "{                       \
        \"ipv4\": \"" + ip_address_ + "\",         \
        \"mac_addr\": \"" + mac_address_ + "\",    \
        \"sensor\": \"" + name_ + "\",             \
        \"data\": " + data + "                     \
    }";
    int httpResponseCode = http.POST(payload_json);
    if (httpResponseCode > 0) {
        sendMessage(String("HTTP Response code: " + httpResponseCode), "INFO");

        // Optionally read response body
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = http.getString();
            if (response.length() > 0 && response.length() < 200) {
                sendMessage(String("  Response: " + response), "INFO");
            }
        }

        http.end();
        return true;
    } else {
        sendMessage(String("[WARN] HTTP Error: " + http.errorToString(httpResponseCode)), "WARN");
        http.end();
        return false;
    }
}

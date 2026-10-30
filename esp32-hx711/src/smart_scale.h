// SPDX-License-Identifier: MIT
#ifndef ESP32HX711_WEIGHT_SENSOR_H
#define ESP32HX711_WEIGHT_SENSOR_H

#include <Arduino.h>
#include <Preferences.h>

#include "types.h"

/// @brief SmartScale describes a smart scale with load cell weight sensor(s).
/// @details Configures the hardware mappings. Use SetSensor before Setup.
class SmartScale {
    // Basic settings
    String          name_;
    String       version_;
    Preferences settings_;
    bool          online_;
    String    ip_address_;
    String   mac_address_;

    // Configuration
    bool           is_quiet_mode_;
    bool         webhook_enabled_;
    unsigned int http_batch_size_;
    uint32_t    interval_between_;

public:
    SmartScale(const char* name, const char* version);
    ~SmartScale() {}

    void Setup(); // maps to sketch::setup()
    void OnLoop(); // maps to sketch::loop()

    void SetSensor(const char*, unsigned short, unsigned short);

    const String&  GetName();
    const String&  GetVersion();
    bool           IsOnline();
    const String&  GetIPAddress();
    const String&  GetMACAddress();

protected:
    void setupWiFiConnection();
    void ensureWiFiConnected();

    bool sendHTTPRequest(const char* url, const char* data);
    const String& consumeBatch();

private:
    // Devices connections
    WeightSensorDevice hx711_module_;

    // Devices states
    bool                 has_module_;
    unsigned long       tare_offset_;
    float        calibration_factor_;

    // Batches states
    std::vector<SensorReading> batch_;
};

#endif
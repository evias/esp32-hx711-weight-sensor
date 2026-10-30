// SPDX-License-Identifier: MIT
#ifndef ESP32HX711_TYPES_H
#define ESP32HX711_TYPES_H

#include <Arduino.h>
#include <HX711.h>

/// @brief  Device describes a device wired with pins to a ESP32 board.
/// @tparam NUM_PINS
template <unsigned int NUM_PINS>
struct Device {
    String         id;
    unsigned short pins[NUM_PINS];
};

/// @brief WeightSensorDevice describes a load cell weight sensor device.
/// @details Contains a HX711 module and a Device for pins hardware mapping.
struct WeightSensorDevice {
    HX711     mod;
    Device<2> dev; // 1=DT ; 2=SCK
};

/// @brief SensorReading describes a sensor reading with timestamp.
struct SensorReading {
    uint32_t  read_at;
    String       name;
    String       unit;
    String      value;

    String ToString() const;
};

/// @brief The ToString method returns a JSON representation of the instance.
inline String SensorReading::ToString() const {
    String str("{                             \
    \"timestamp\": " + String(read_at) + ",             \
    \"name\": \"" + name.c_str() + "\",                 \
    \"value\": \"" + value.c_str() + "\",               \
    \"unit_of_measurement\": \"" + unit.c_str() + "\",  \
}");
    return str;
}
#endif
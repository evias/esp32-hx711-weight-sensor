# Firmware for ESP32 with HX711 Load Cell Weight Sensors 

An arduino sketch to run periodical weight reports using an ESP32 development
board and HX711 module with load cell weight sensors.

- **Tester**: A `ESP32+HX711` diagnostic firmware: [`esp32-board-info`](esp32-board-info/esp32-board-info.ino).
- **Firmware**: A `ESP32+HX711` production firmware: [`esp32-hx711`](esp32-hx711/esp32-hx711.ino).

## Developer notes

This repository contains arduino sketches that may be built/uploaded using Arduino IDE
or VSCode. I personally use VSCode with the added `vscode-arduino` community extension.

When building with VSCode, the `.vscode/arduino.json` must be updated to reflect
the build configuration for each *separate* arduino sketch, e.g.:

```bash
# Prepare for building the PRODUCTION firmware.
# ESP32-DEVKIT-V1 + HX711 module + Load cell weight sensors
cp .vscode/arduino.PROD.json .vscode/arduino.json

# OR prepare for building the TESTING firmware.
cp .vscode/arduino.TEST.json .vscode/arduino.json
```

After having done that, enable the `vscode-arduino` extension for your workspace
and use the `Arduino: Verify` and `Arduino: Upload` actions from the command
palette to build and/or upload the firmware to your chosen board.

The default `.vscode/arduino.json` file contains building instruction for `production`.

### Webhook URL

The production firmware includes a `WEBHOOK_URL` build parameter. When omitted,
the value for this webhook URL is set to `http://localhost:8123/api/webhook/sensors`.

The destination server will receive a JSON payload over a HTTP `POST` request, which
contains the following structure:

```json
{
    "ipv4": "...",
    "mac_addr": "...",
    "sensor": "sensor.weight_sensor_aabb",
    "data": [
        { "timestamp": 321, "name": "weight", "value": "100", "unit_of_measurement": "g" },
        { "timestamp": 322, "name": "weight", "value": "101", "unit_of_measurement": "g" },
    ],
}
```

## Hardware wiring

| ESP32 Pin | GPIO Number | Connected To |
|-----------|-------------|--------------|
| **GND** | - | HX711 GND |
| **5V** | - | HX711 VCC |
| **RX2** | GPIO 16 | HX711 DT |
| **D4** | GPIO 4 | HX711 SCK |

## License

Copyright 2026 Grégory Saive <greg@evi.as> for re:Software S.L. (resoftware.es).

Licensed under the [MIT License](./LICENSE).
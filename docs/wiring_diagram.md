# Wiring Diagram for Smoking Timer Box

This document provides a wiring diagram for connecting the components of the Smoking Timer Box project. Below is a description of how to wire the ESP32 S3 Nano, OLED display, DS3218MG servo, and push button.

## Components

1. **ESP32 S3 Nano**
2. **OLED Display (128x64)**
3. **DS3218MG Servo**
4. **Push Button**

## Wiring Connections

### ESP32 S3 Nano to OLED Display
- **VCC** (OLED) to **3.3V** (ESP32)
- **GND** (OLED) to **GND** (ESP32)
- **SDA** (OLED) to **GPIO 21** (ESP32)
- **SCL** (OLED) to **GPIO 22** (ESP32)

### ESP32 S3 Nano to DS3218MG Servo
- **Signal** (Servo) to **GPIO 23** (ESP32)
- **VCC** (Servo) to **5V** (ESP32)
- **GND** (Servo) to **GND** (ESP32)

### ESP32 S3 Nano to Push Button
- One terminal of the **Push Button** to **GPIO 19** (ESP32)
- The other terminal of the **Push Button** to **GND** (ESP32)

## Diagram

Please refer to the attached wiring diagram image for a visual representation of the connections.

## Notes
- Ensure that all connections are secure to prevent any intermittent issues.
- Use appropriate resistors if necessary for the push button to avoid floating pin states.

This wiring setup will allow the components to function together effectively in the Smoking Timer Box project.
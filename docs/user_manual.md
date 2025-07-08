# User Manual for Quit Smoking Timer Box

## Introduction
The Quit Smoking Timer Box is designed to help individuals manage their smoking habits by providing a physical mechanism to control access to cigarettes. This user manual will guide you through the setup, usage, and maintenance of the device.

## Components
- ESP32 S3 Nano
- OLED Display (128x64)
- DS3218MG Servo
- Push Button
- 3D Printed Box (Base and Lid)
- Connecting Wires

## Setup Instructions

### 1. Assembly
- **3D Printing**: Print the box base, lid, and servo mount using the provided STL files located in the `3d_models` directory.
- **Component Placement**: Securely place the OLED display, servo, and push button inside the printed box. Ensure that the servo is mounted correctly to control the locking mechanism.

### 2. Wiring
Refer to the `wiring_diagram.md` document in the `docs` directory for detailed wiring instructions. Ensure all connections are secure to avoid any malfunctions.

### 3. Software Installation
- Install PlatformIO on your development environment.
- Clone the repository containing the project files.
- Open the `platformio.ini` file and ensure all dependencies are correctly listed.
- Upload the code from `src/main.cpp` to the ESP32 S3 Nano using PlatformIO.

## Using the Quit Smoking Timer Box

### 1. Initial Setup
- Power on the device. The OLED display will initialize and show the current status.
- Set the desired countdown timer duration using the configuration settings in `include/config.h`.

### 2. Locking Mechanism
- The box will automatically lock after the timer starts. The servo will engage to secure the lid.
- The OLED display will show the countdown timer and the lock status.

### 3. Unlocking the Box
- To unlock the box, press the push button located inside the box. This will temporarily unlock the lid for a specified duration.
- The OLED display will indicate the unlock status and the remaining time before the box locks again.

### 4. Monitoring Progress
- The OLED display will continuously update to show the countdown timer and whether the box is locked or unlocked.
- Use this feedback to help manage your smoking habits effectively.

## Maintenance
- Regularly check the wiring and connections to ensure everything is functioning correctly.
- Keep the OLED display clean and free from obstructions for optimal visibility.
- If the servo mechanism becomes unresponsive, check the power supply and connections.

## Conclusion
The Quit Smoking Timer Box is a valuable tool for those looking to control their smoking habits. By following this user manual, you can effectively set up and utilize the device to aid in your journey towards quitting smoking. For further assistance, refer to the `README.md` for troubleshooting tips and additional resources.
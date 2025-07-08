# Assembly Guide for the Quit Smoking Timer Box

## Introduction
This guide provides step-by-step instructions on how to assemble the Quit Smoking Timer Box using the ESP32 S3 Nano, OLED display, DS3218MG servo, and a push button. The goal of this project is to help individuals control their smoking habits by locking away cigarettes for a predetermined time.

## Components Required
- ESP32 S3 Nano
- OLED Display (128x64)
- DS3218MG Servo
- Push Button
- Jumper Wires
- Breadboard (optional for testing)
- 3D Printed Box Parts:
  - Box Base
  - Box Lid
  - Servo Mount

## Assembly Steps

### Step 1: 3D Printing the Box
1. Download the STL files from the `3d_models` directory.
2. Use a 3D printer to print the following components:
   - `box_base.stl`
   - `box_lid.stl`
   - `servo_mount.stl`
3. Ensure that the printed parts fit together properly before proceeding.

### Step 2: Preparing the Electronics
1. Gather all electronic components:
   - ESP32 S3 Nano
   - OLED Display
   - DS3218MG Servo
   - Push Button
   - Jumper Wires

### Step 3: Wiring the Components
1. **OLED Display**:
   - Connect the VCC pin of the OLED to the 3.3V pin on the ESP32.
   - Connect the GND pin of the OLED to a GND pin on the ESP32.
   - Connect the SCL pin of the OLED to GPIO 22 on the ESP32.
   - Connect the SDA pin of the OLED to GPIO 21 on the ESP32.

2. **Servo**:
   - Connect the red wire (VCC) of the servo to the 5V pin on the ESP32.
   - Connect the brown wire (GND) of the servo to a GND pin on the ESP32.
   - Connect the yellow wire (signal) of the servo to GPIO 23 on the ESP32.

3. **Push Button**:
   - Connect one terminal of the push button to GPIO 19 on the ESP32.
   - Connect the other terminal of the push button to GND.

### Step 4: Assembling the Box
1. Place the ESP32, OLED display, and servo into the printed box base.
2. Secure the servo using the servo mount.
3. Ensure that the push button is accessible from the interior of the box.
4. Attach the box lid securely to the base.

### Step 5: Uploading the Code
1. Open PlatformIO and load the project.
2. Connect the ESP32 to your computer via USB.
3. Compile and upload the code from `src/main.cpp` to the ESP32.

### Step 6: Testing the Assembly
1. Power on the box and observe the OLED display.
2. Test the push button to ensure it unlocks the box when pressed.
3. Verify that the servo locks and unlocks the box as intended.

## Conclusion
Once assembled, the Quit Smoking Timer Box will help users manage their smoking habits by providing a physical barrier to access cigarettes. Follow the user manual for detailed instructions on how to use the timer and locking mechanism effectively.
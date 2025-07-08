# 🚭 Quit Smoking Timer Box

An open-source, ESP32-powered timer box designed to help people control their smoking habits or quit entirely. The box features a servo-controlled locking mechanism, OLED display, and a beautiful web interface for configuration.

![Quit Smoking Timer Box](https://img.shields.io/badge/Status-Active%20Development-green) ![License](https://img.shields.io/badge/License-MIT-blue) ![Platform](https://img.shields.io/badge/Platform-ESP32--S3-red)

## 📋 Table of Contents

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Installation](#-installation)
- [Assembly Guide](#-assembly-guide)
- [Usage](#-usage)
- [Web Interface](#-web-interface)
- [3D Models](#-3d-models)
- [Configuration](#-configuration)
- [Contributing](#-contributing)
- [License](#-license)

## ✨ Features

### Core Functionality
- **Servo-controlled locking mechanism** with DS3218MG servo support
- **128x64 OLED display** showing countdown timer and status
- **WiFi Access Point** for easy configuration without additional apps
- **Emergency unlock button** located inside the box
- **Multiple timer modes** for different quitting strategies

### Timer Modes
1. **Fixed Interval** - Set a specific time between unlocks
2. **Gradual Reduction** - Automatically increase intervals over time
3. **Complete Quit Mode** - Progressive lockout to eliminate smoking
4. **Emergency Only** - Only emergency unlocks allowed

### Smart Features
- **Progress tracking** with statistics and achievements
- **Emergency unlock limits** with penalty system
- **Data persistence** using ESP32 preferences
- **Beautiful web interface** with calm, organic design
- **Real-time countdown** on both display and web interface
- **Multiple user profiles** support

### Web Interface Features
- 📊 **Real-time status monitoring**
- ⏱️ **Timer configuration** with dropdown menus
- 🎯 **Quick action buttons** for manual control
- 📈 **Progress statistics** showing money saved and cigarettes avoided
- 🚨 **Emergency unlock** with penalty tracking
- 🔄 **Progress reset** functionality
- 🧪 **Hardware testing** tools

## 🔧 Hardware Requirements

### Required Components
- **ESP32-S3 Nano** (or compatible ESP32-S3 board)
- **0.96" OLED Display** (128x64, I2C, SSD1306)
- **DS3218MG Servo** (or compatible high-torque servo)
- **Push Button** (for emergency unlock)
- **Jumper wires** and **breadboard** for prototyping

### Optional Components
- **Status LEDs** for visual feedback
- **Buzzer** for audio alerts
- **External power supply** for servo (recommended for DS3218MG)

### Power Requirements
- ESP32-S3: 3.3V (via USB or external supply)
- Servo: 6V-7.4V (external supply recommended)
- OLED Display: 3.3V

## 📦 Installation

### Prerequisites
- **PlatformIO IDE** (VS Code extension recommended)
- **Git** for version control
- **3D Printer** (for box enclosure)

### Step 1: Clone the Repository
```bash
git clone https://github.com/your-username/QuitSmokingBox.git
cd QuitSmokingBox
```

### Step 2: Install Dependencies
PlatformIO will automatically install all required libraries when you build the project.

### Step 3: Configure Hardware Pins
Edit `include/config.h` to match your wiring:

```cpp
// Pin Definitions (adjust as needed)
#define CONFIG_PIN_SDA 21      // OLED SDA
#define CONFIG_PIN_SCL 22      // OLED SCL  
#define CONFIG_PIN_SERVO 18    // Servo signal
#define CONFIG_PIN_BUTTON 19   // Emergency button
```

### Step 4: Upload Firmware
```bash
# Build and upload firmware
pio run --target upload

# Upload web interface files to SPIFFS
pio run --target uploadfs

# Monitor serial output
pio device monitor
```

## 🔧 Assembly Guide

### Wiring Diagram

```
ESP32-S3 Nano    OLED Display    DS3218MG Servo    Push Button
     21    -->       SDA
     22    -->       SCL
     3V3   -->       VCC
     GND   -->       GND
     
     18    -->                      Signal (Orange)
     GND   -->                      Ground (Brown)
     VIN   -->                      Power (Red)*
     
     19    -->                                      One terminal
     GND   -->                                      Other terminal

* Use external 6V supply for servo power, share GND with ESP32
```

### Step-by-Step Assembly

1. **Prepare the ESP32-S3 Nano**
   - Connect USB cable for programming
   - Verify board selection in PlatformIO

2. **Connect OLED Display**
   - Wire I2C connections (SDA, SCL)
   - Connect power (3.3V, GND)
   - Test display initialization

3. **Install Servo Motor**
   - Mount servo in the box mechanism
   - Connect signal wire to GPIO 18
   - Use external power supply for reliable operation

4. **Add Emergency Button**
   - Install button inside the box
   - Wire between GPIO 19 and GND
   - Add pull-up resistor if needed

5. **3D Print Enclosure**
   - Print all STL files from `3d_models/` directory
   - Use PLA or PETG plastic
   - Post-process for smooth fit

### Detailed Assembly Instructions
See [docs/assembly_guide.md](docs/assembly_guide.md) for complete assembly instructions with photos.

## 🎯 Usage

### Initial Setup
1. **Power on** the device
2. **Connect to WiFi** Access Point:
   - SSID: `QuitSmokingBox`
   - Password: `quit2024`
3. **Open web browser** and navigate to: `192.168.4.1`
4. **Configure** your timer settings

### Timer Modes Explained

#### Fixed Interval Mode
- Set a specific time between unlocks (e.g., 30 minutes)
- Box locks automatically after each use
- Consistent intervals help build routine

#### Gradual Reduction Mode
- Start with shorter intervals
- Automatically increases time between unlocks
- Gentle approach to reducing smoking

#### Complete Quit Mode
- Progressive lockout system
- Each use increases next lockout period
- Designed for complete cessation

#### Emergency Only Mode
- Box stays locked except for emergencies
- Limited emergency unlocks per day
- For advanced users committed to quitting

### Daily Operation
1. **Use the box** when you need access
2. **Monitor progress** via OLED display
3. **Check statistics** on web interface
4. **Emergency unlock** available from inside if needed

## 🌐 Web Interface

The web interface provides a beautiful, modern dashboard for configuring and monitoring your quit smoking progress.

### Main Features

#### Status Dashboard
- **Real-time countdown** with smooth animations
- **Current box state** (Locked/Unlocked/Countdown)
- **Today's usage count** and smoke-free days
- **WiFi connection status**

#### Configuration Panel
- **Timer mode selection** with detailed descriptions
- **Interval settings** with validation
- **Daily limits** and emergency settings
- **Save/restore** configurations

#### Quick Actions
- **Manual unlock** for immediate access
- **Emergency unlock** with penalty tracking
- **System reset** with confirmation dialog
- **Servo testing** for troubleshooting

#### Progress Statistics
- **Money saved** calculations
- **Cigarettes avoided** counter
- **Usage tracking** and streaks
- **Visual progress** indicators

### Responsive Design
- **Mobile-friendly** interface
- **Touch-optimized** controls
- **Calm color scheme** for reduced stress
- **Accessibility** features included

## 🎨 3D Models

All STL files are included in the `3d_models/` directory:

### Print Settings
- **Layer height**: 0.2mm
- **Infill**: 20%
- **Supports**: Yes (for overhangs)
- **Print speed**: 50mm/s

### Files Included
- `box_base.stl` - Main box container
- `box_lid.stl` - Servo-controlled lid
- `servo_mount.stl` - Mounting bracket for DS3218MG

### Customization
- Files are designed for standard cigarette packs
- Modify dimensions in your CAD software if needed
- Consider printing in different colors for personalization

## ⚙️ Configuration

### WiFi Settings
```cpp
#define AP_SSID "QuitSmokingBox"
#define AP_PASSWORD "quit2024"
```

### Timer Settings
```cpp
#define MIN_TIMER_MINUTES 1
#define MAX_TIMER_MINUTES 1440  // 24 hours
#define DEFAULT_TIMER_MINUTES 30
```

### Emergency Settings
```cpp
#define EMERGENCY_UNLOCK_PENALTY 15  // minutes
#define MAX_EMERGENCY_UNLOCKS_PER_DAY 3
```

### Hardware Pins
All pin assignments can be modified in `include/config.h` to match your wiring.

## 🤝 Contributing

We welcome contributions! Here's how you can help:

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Areas for Contribution
- **New timer modes** and algorithms
- **Mobile app** development
- **Additional hardware** support
- **Translations** for international users
- **Documentation** improvements
- **3D model** variations

### Code Style
- Follow existing code formatting
- Comment complex functions
- Use meaningful variable names
- Test on actual hardware when possible

## 📞 Support

### Troubleshooting
- Check [docs/user_manual.md](docs/user_manual.md) for common issues
- Verify wiring connections
- Monitor serial output for error messages
- Test individual components separately

### Getting Help
- Open an issue on GitHub
- Check existing issues for solutions
- Provide detailed error descriptions
- Include serial monitor output when relevant

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Open Source Philosophy
This project is completely open source because:
- **Health is a human right** - tools for quitting smoking should be freely available
- **Community development** leads to better solutions
- **Transparency** builds trust in health-related devices
- **Customization** allows adaptation to individual needs

## 🙏 Acknowledgments

- **Community feedback** for feature suggestions
- **Open source libraries** that made this possible
- **Health organizations** promoting smoking cessation
- **Makers and tinkerers** who share knowledge freely

## 🚀 Future Roadmap

### Planned Features
- [ ] **Mobile app** for remote monitoring
- [ ] **Cloud sync** for multi-device support
- [ ] **Social features** for community support
- [ ] **Advanced analytics** and insights
- [ ] **Integration** with health apps
- [ ] **Voice control** via Alexa/Google
- [ ] **Biometric** locks for security

### Hardware Improvements
- [ ] **Battery power** option
- [ ] **Solar charging** capability
- [ ] **Smaller form factor** designs
- [ ] **Multiple compartment** support
- [ ] **Tamper detection** sensors

---

**Made with ❤️ for a healthier lifestyle**

*If this project helps you or someone you know quit smoking, please consider starring the repository and sharing it with others who might benefit.*

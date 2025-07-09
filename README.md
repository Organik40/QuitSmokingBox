# 🚭 Quit Smoking Timer Box

An open-source, ESP32-powered timer box designed to help people control their smoking habits or quit entirely. The box features a servo-controlled locking mechanism, OLED display, and a beautiful web interface for configuration.

![Quit Smoking Timer Box](https://img.shields.io/badge/Status-Active%20Development-orange) ![License](https://img.shields.io/badge/License-MIT-blue) ![Platform](https://img.shields.io/badge/Platform-ESP32--S3-red) ![Features](https://img.shields.io/badge/Features-AI%20Ready-purple)

## 🚀 Latest Updates (v2.0)

### ✅ Core Features Implemented
- **🤖 AI Emergency Gatekeeper System** - Complete implementation with 4 personalities, reflection questions, and breathing exercises
- **🌐 Real-time WebSocket Updates** - Live status updates without page refresh
- **📱 Progressive Web App (PWA)** - Mobile app installation support with offline capabilities
- **🔒 Network-based Security** - Public WiFi blocking and network restrictions
- **📊 Enhanced Statistics** - Money saved calculations and progress tracking
- **⚡ Advanced Timer Modes** - Gradual reduction and complete quit algorithms
- **🛡️ Security Framework** - Network-based access control system
- **⚙️ Complete Settings Interface** - Web-based configuration for all features

### 🔧 Features Coming Soon
- **🎨 3D Enclosure Models** - Professional housing for all components *(Design in progress)*
- **🤖 Advanced AI Personalities** - Enhanced conversational depth *(Framework complete, expanding responses)*
- **🧠 Enhanced AI Intelligence** - Context-aware responses and learning capabilities *(In development)*
- **☁️ Cloud Sync & Backup** - Multi-device progress synchronization *(Planned)*
- **📱 Native Mobile App** - Dedicated iOS/Android application *(Planned)*
- **🔍 Advanced Analytics** - Detailed usage patterns and predictive insights *(Planned)*
- **👥 Social Features** - Community support and progress sharing *(Planned)*
- **🔗 Health App Integration** - Apple Health, Google Fit connectivity *(Planned)*
- **🎮 Gamification** - Achievement system and challenges *(Planned)*

### 🛠️ Hardware Features Coming Soon
- **🎨 3D Printed Enclosure** - Professional housing design *(In development)*
- **🔍 Tamper Detection** - Physical security enhancements *(Hardware pending)*
- **🔋 Battery Power Support** - Standalone operation capability *(Hardware pending)*
- **☀️ Solar Charging** - Eco-friendly power option *(Hardware pending)*
- **🎤 Voice Control** - Alexa/Google Assistant integration *(Planned)*
- **🔐 Biometric Lock** - Fingerprint security option *(Hardware pending)*
- **📡 Cellular Connectivity** - 4G/5G option for remote areas *(Hardware pending)*n open-source, ESP32-powered timer box designed to help people control their smoking habits or quit entirely. The box features a servo-controlled locking mechanism, OLED display, and a beautiful web interface for configuration.

![Quit Smoking Timer Box](https://img.shields.io/badge/Status-Active%20Development-orange) ![License](https://img.shields.io/badge/License-MIT-blue) ![Platform](https://img.shields.io/badge/Platform-ESP32--S3-red) ![Features](https://img.shields.io/badge/Features-AI%20Ready-purple)

## 🚀 Latest Updates (v2.0 - In Development)# 🚀 Latest Updates (v2.0 - In Development)it Smoking Timer Box

An open-source, ESP32-powered timer box designed to help people control their smoking habits or quit entirely. The box features a servo-controlled locking mechanism, OLED display, and a beautiful web interface for configuration.

![Quit Smoking Timer Box](https://img.shields.io/badge/Status-Active%20Development-orange) ![License](https://img.shields.io/badge/License-MIT-blue) ![Platform](https://img.shields.io/badge/Platform-ESP32--S3-red) ![Features](https://img.shields.io/badge/Features-AI%20Ready-purple)

## � Latest Updates (v2.0)

### ✨ Core Features Implemented
- **🤖 AI Emergency Gatekeeper Framework** - Backend ready, basic OpenAI integration and simple AI available
- **🌐 Real-time WebSocket Updates** - Live status updates without page refresh
- **📱 Progressive Web App (PWA)** - Mobile app installation support with offline capabilities
- **🔒 Network-based Security** - Public WiFi blocking and network restrictions
- **📊 Enhanced Statistics** - Money saved calculations and progress tracking
- **⚡ Advanced Timer Modes** - Gradual reduction and complete quit algorithms
- **🛡️ Security Framework** - Network-based access control system
- **⚙️ Complete Settings Interface** - Web-based configuration for all features

### 🔧 Features Coming Soon
- **🤖 Complete AI Personalities** - Full conversational AI with multiple personalities *(Framework ready)*
- **☁️ Cloud Sync & Backup** - Multi-device progress synchronization *(Planned)*
- **� Mobile App Companion** - Dedicated mobile application *(Planned)*
- **🔍 Advanced Analytics** - Detailed usage patterns and insights *(Planned)*
- **👥 Social Features** - Community support and progress sharing *(Planned)*
- **�️ Hardware Tamper Detection** - Physical security enhancements *(Hardware pending)*
- **🔋 Battery Power Support** - Standalone operation capability *(Hardware pending)*

### 🎯 Implementation Status
Current development status of major features:
1. ✅ **Core Timer System** - Fully functional with all 7 modes
2. ✅ **Web Interface** - Complete with real-time updates  
3. ✅ **PWA Support** - Ready for mobile installation with offline support
4. ✅ **AI Emergency System** - Fully implemented with enhanced personalities, reflection questions, and breathing exercises
5. ✅ **Network Security** - Operational with configurable restrictions
6. ✅ **Statistics Tracking** - Enhanced progress monitoring active
7. ✅ **Settings Management** - Complete configuration interface available
8. ✅ **Advanced AI Features** - Complete implementation with 4 personalities, reflection system, and interactive breathing exercises

---

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Installation](#-installation)
- [Assembly Guide](#-assembly-guide)
- [Usage](#-usage)
- [Web Interface](#-web-interface)
- [3D Models](#-3d-models-coming-soon)
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
5. **Daily Schedule** - Unlock once per day at a specific time (default 22:00)
6. **Weekly Schedule** - Unlock once per week on a chosen day and time
7. **Custom Schedule** - Flexible scheduling for personalized quit plans

### Smart Features
- **Progress tracking** with statistics and achievements
- **Emergency unlock limits** with penalty system and 5-minute cooling-off period
- **AI Emergency Gatekeeper** (optional) - 10-minute conversation system to dissuade emergency unlocks
- **Data persistence** using ESP32 preferences
- **Beautiful web interface** with calm, organic design
- **Real-time countdown** on both display and web interface
- **Multiple user profiles** support
- **Scheduled unlocks** with NTP time synchronization
- **Intelligent countdown display** showing days/hours for long periods
- **Web-based schedule configuration** with time picker and day selection
- **WiFi connectivity** with configurable network settings
- **Network-based emergency restrictions** (block on public WiFi, allow on safe networks)

### Web Interface Features
- 📊 **Real-time status monitoring**
- ⏱️ **Timer configuration** with dropdown menus
- 🎯 **Quick action buttons** for manual control
- 📈 **Progress statistics** showing money saved and cigarettes avoided
- 🚨 **Emergency unlock** with 5-minute cooling-off period or optional AI gatekeeper
- 🤖 **AI Emergency Gatekeeper** - Intelligent conversation system to help resist cravings
- ⚙️ **Settings page** for WiFi, AI configuration, and network security
- 🔄 **Progress reset** functionality
- 🧪 **Hardware testing** tools

### AI Emergency Gatekeeper (Fully Implemented) ✅
- **10-minute minimum conversation** before emergency unlock is allowed ✅
- **Multiple AI personalities**: Supportive Coach, Strict Counselor, Understanding Friend, Professional Therapist ✅
- **Intelligent responses** based on user triggers (stress, boredom, anger, habits) ✅
- **Coping strategies** including breathing exercises and alternative activities ✅
- **Network restrictions** - can block emergency unlocks on public WiFi or specific networks ✅
- **Configurable providers**: OpenAI GPT ✅, Local AI *(Coming soon)*, Simple rule-based responses ✅
- **Reflection questions** to encourage deeper thinking about the craving ✅
- **Interactive breathing exercises** with guided 4-7-8 breathing technique ✅
- **Test mode** to try the AI system without actual emergency unlock ✅

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

### 3D Printed Enclosure *(Coming Soon)*
- Custom-designed STL files for a complete box enclosure
- Servo mounting bracket for secure installation
- Elegant design with easy access to components
- Files will be available in the `3d_models/` directory when ready

## 📦 Installation

### Prerequisites
- **PlatformIO IDE** (VS Code extension recommended)
- **Git** for version control
- **3D Printer** *(optional - for enclosure when STL files are released)*

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

5. **3D Print Enclosure** *(Optional - Coming Soon)*
   - STL files for custom enclosure will be available soon
   - Current prototyping can be done with breadboard
   - Servo can be temporarily mounted for testing

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
4. **Emergency unlock** available with built-in deterrents

### Emergency Unlock Modes

#### Normal Mode (Default) ✅
- **5-minute cooling-off period** with countdown timer
- **Reflection prompts** to encourage reconsidering *(Coming soon)*
- **Simple but effective** delay to reduce impulsive unlocks
- **No additional setup required**

#### AI Gatekeeper Mode (Partially Available) ⚙️
- **10-minute interactive conversation** with AI ✅
- **Basic AI responses** based on your triggers and situation ✅
- **Simple coping strategies** and alternatives ✅
- **Enhanced personalities** and deeper conversations *(Coming soon)*
- **Advanced breathing exercises** and mindfulness techniques *(Coming soon)*
- **Network-based restrictions** for high-risk environments ✅
- **Requires configuration** in Settings page ✅

Both modes include penalty time added to your next timer interval.

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

## 🎨 3D Models *(Coming Soon)*

The 3D enclosure models are currently in development and will be released soon.

### Planned Features
- **Complete enclosure design** for professional appearance
- **Servo mounting system** for secure DS3218MG installation  
- **OLED display window** for easy viewing
- **Access ports** for charging and programming
- **Security features** to prevent tampering

### Print Settings *(When Available)*
- **Layer height**: 0.2mm
- **Infill**: 20%
- **Supports**: Yes (for overhangs)
- **Print speed**: 50mm/s
- **Material**: PLA or PETG recommended

### Files To Be Included
- `box_base.stl` - Main box container
- `box_lid.stl` - Servo-controlled lid
- `servo_mount.stl` - Mounting bracket for DS3218MG
- `display_mount.stl` - OLED display frame

### Current Status
- **Design Phase**: In progress
- **Prototyping**: Breadboard version functional
- **Release**: Coming in next update

For now, you can prototype the system using a breadboard and temporary servo mounting.

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
#define DEFAULT_EMERGENCY_DELAY_MINUTES 5  // cooling-off period
#define AI_EMERGENCY_DELAY_MINUTES 10      // AI gatekeeper session
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

### 🔧 Short-term Development (Coming Soon)
- [ ] **3D Enclosure Models** - Complete STL files for professional housing
- [ ] **Enhanced AI personalities** - Complete conversational AI implementation (Framework ready)
- [ ] **Advanced AI intelligence** - Context-aware responses and learning
- [ ] **Local AI integration** - Ollama and other local AI providers
- [ ] **Reflection prompts** - Deep questions for craving analysis (Framework ready)
- [ ] **Gamification elements** - Achievement system and challenges

### 📱 Medium-term Features (Planned)
- [ ] **Native mobile app** - iOS and Android applications
- [ ] **Cloud sync** - Multi-device progress synchronization
- [ ] **Advanced analytics** - Detailed usage patterns and insights
- [ ] **Social features** - Community support and progress sharing
- [ ] **Health app integration** - Apple Health, Google Fit connectivity
- [ ] **Voice control** - Alexa/Google Assistant integration

### 🛠️ Long-term Hardware Improvements (Hardware Pending)
- [ ] **Battery power** - Standalone operation capability
- [ ] **Solar charging** - Eco-friendly power option
- [ ] **Tamper detection** - Physical security sensors
- [ ] **Biometric locks** - Fingerprint security
- [ ] **Smaller form factor** - Compact designs
- [ ] **Multiple compartments** - Support for different items
- [ ] **Cellular connectivity** - 4G/5G for remote areas

---

**Made with ❤️ for a healthier lifestyle**

*If this project helps you or someone you know quit smoking, please consider starring the repository and sharing it with others who might benefit.*

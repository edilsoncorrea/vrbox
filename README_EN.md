# 🎮 VRBOX ESP32-C3 BLE Client - RC Car Control

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--C3-orange)](https://platformio.org/)
[![Arduino](https://img.shields.io/badge/Framework-Arduino-blue)](https://www.arduino.cc/)
[![BLE](https://img.shields.io/badge/Protocol-Bluetooth%20LE-lightblue)](https://www.bluetooth.com/)
[![Status](https://img.shields.io/badge/Status-WORKING-brightgreen)](https://github.com/edilsoncorrea/vrbox)

## 📋 Description

This project implements a BLE (Bluetooth Low Energy) client to connect an **ESP32-C3** to a **VRBOX** joystick, transforming it into a remote control for **RC cars** with servo and ESC. The system receives joystick data in real-time and converts it to PWM signals compatible with standard RC equipment.

## ✨ Features

### 🔗 BLE Communication
- ✅ **Automatic connection** to VRBOX devices
- ✅ **Specific parsing** of VRBOX HID protocols
- ✅ **Real-time reception** of joystick data (X/Y)
- ✅ **Detection of triggers** and A/B/C/D buttons
- ✅ **Automatic reconnection** on signal loss
- ✅ **Smart filters** to avoid incorrect connections

### 🚗 RC Car Control
- ✅ **50Hz PWM signals** compatible with standard servos/ESCs
- ✅ **12-bit resolution** (4096 levels) for precise control
- ✅ **Optimized mapping** for VRBOX real range (-36 to +36)
- ✅ **Deadband zone** to avoid jitter
- ✅ **Smoothing filter** for smooth transitions
- ✅ **Dedicated outputs** - GPIO4 (servo) and GPIO5 (ESC)

## 🛠️ Hardware

### Required Components
- **ESP32-C3-DevKitM-1** (main microcontroller)
- **VRBOX** (BLE joystick with name "VR BOX")
- **Servo** for steering control
- **ESC** (Electronic Speed Controller) for motor control
- **RC Car** or compatible chassis

### Technical Specifications
- **Microcontroller**: ESP32-C3 (160MHz, 320KB RAM, 4MB Flash)
- **Protocol**: HID over GATT (UUID 1812)
- **PWM**: 50Hz, 1000-2000μs (RC standard)
- **Control range**: 5% to 10% duty cycle
- **Latency**: <100ms (real-time)

## 📡 Pinout and Connections

### ESP32-C3 DevKitM-1
```
                    ┌─────────────────┐
                    │  ESP32-C3-DevKitM-1  │
                    │                 │
              3V3   │ 1  ┌─────────┐ 21 │  GPIO21
              GND   │ 2  │         │ 20 │  GPIO20  
            GPIO0   │ 3  │  ESP32  │ 19 │  GPIO19
            GPIO1   │ 4  │   C3    │ 18 │  GPIO18
            GPIO2   │ 5  │         │ 10 │  GPIO10
            GPIO3   │ 6  └─────────┘  9 │  GPIO9
          ➤ GPIO4   │ 7     SERVO     8 │  GPIO8
          ⚡ GPIO5   │ 8     ESC       7 │  GPIO7
            GPIO6   │ 9               6 │  GPIO6
                    └─────────────────┘
```

### System Connections
```
┌─────────────┐    BLE     ┌─────────────┐    PWM     ┌─────────────┐
│             │◄──────────►│             │───GPIO4───►│    SERVO    │
│   VRBOX     │            │  ESP32-C3   │            │ (Steering)  │
│ (Joystick)  │            │             │───GPIO5───►│     ESC     │
│             │            │             │            │   (Motor)   │
└─────────────┘            └─────────────┘            └─────────────┘
```

### Detailed Wiring Diagram
```
VRBOX                    ESP32-C3                    RC CAR
┌─────────┐             ┌─────────┐                ┌─────────┐
│ Stick X │─(BLE HID)──►│ GPIO4   │─(PWM 50Hz)────►│ Servo   │
│ Stick Y │             │ GPIO5   │─(PWM 50Hz)────►│ ESC     │
│Triggers │             │ 3V3     │─(+3V3)────────►│ Servo   │
│Buttons  │             │ GND     │─(GND)─────────►│ Servo   │
└─────────┘             └─────────┘                │ ESC     │
                                                   └─────────┘
```

## ⚙️ Setup

### 1. Environment Preparation
```bash
# Install PlatformIO CLI
pip install platformio

# Clone repository
git clone https://github.com/edilsoncorrea/vrbox.git
cd vrbox

# Compile project
pio run

# Upload to ESP32-C3
pio run --target upload
```

### 2. VRBOX Configuration
1. **Turn on VRBOX** (press power button)
2. **Activate Mouse mode**: Press `@ + D` simultaneously
3. **Check LED**: Should blink indicating active BLE mode
4. **Wait for connection**: ESP32 will find automatically

### 3. System Calibration
```cpp
// Adjust deadband if needed
#define DEADBAND 4      // ±4 = ~10% of range (-36~+36)

// Adjust smoothing
#define SMOOTHING_FACTOR 0.3f  // 0.0 = no filter, 0.9 = very smooth

// Check your VRBOX range (via Serial Monitor)
// X: usually 0 to +36
// Y: usually -36 to +36
```

## 🚀 How to Use

### 1. Upload and Initialization
```bash
# Upload firmware
pio run --target upload

# Monitor Serial (optional)
pio device monitor --baud 115200
```

### 2. Operation
1. **Turn on ESP32-C3** - wait for initialization
2. **Activate VRBOX** - press `@ + D` for Mouse mode
3. **Wait for connection** - ESP32 LED will light up
4. **Test controls**:
   - **Stick X** → Servo direction (left/right)
   - **Stick Y** → Motor speed (forward/reverse)
   - **Triggers** → Additional functions

### 3. PWM Signal Interpretation
```
Joystick Position   →    PWM Signal    →    Action
─────────────────────────────────────────────────
Center (0)          →    1500μs       →    Neutral
Maximum (+36)       →    2000μs       →    Right/Forward
Minimum (-36)       →    1000μs       →    Left/Reverse
```

## 📊 Monitoring and Debug

### Serial Monitor
```
🔍 Device found: 'VR BOX'
✅ VRBOX found!
🎯 Connecting to VRBOX...
✅ Found at least one service with notifications!

📨 Data received [4 bytes]: 00 1A FF 00
🕹️  JOYSTICK - X: 26 (0.20), Y: -1 (-0.01), Triggers: 0x00

🎛️  Channel 0: 1625μs -> PWM: 332 (8.1%)
🎛️  Channel 1: 1486μs -> PWM: 304 (7.4%)
🚗 RC Control - X:26->1625μs, Y:-1->1486μs
```

### Status Codes
- `📊 Active polling` - Stable connection
- `🔄 Executing activation commands` - Keeping VRBOX active
- `❌ Polling error` - Connection problems
- `✅ Connection OK` - System working

## 🔧 Customization

### Adjust Sensitivity
```cpp
// Modify deadband
#define DEADBAND 2      // More sensitive (smaller value)
#define DEADBAND 8      // Less sensitive (larger value)

// Modify smoothing
#define SMOOTHING_FACTOR 0.1f  // Faster response
#define SMOOTHING_FACTOR 0.7f  // Smoother response
```

### Change PWM Pins
```cpp
// Change output pins
#define SERVO_PIN 6     // New pin for servo
#define ESC_PIN 7       // New pin for ESC

// Change PWM channels
#define SERVO_CHANNEL 2
#define ESC_CHANNEL 3
```

### Customize PWM Range
```cpp
// For servos/ESCs with different ranges
#define PWM_MIN 800     // 0.8ms
#define PWM_MAX 2200    // 2.2ms
```

## ❗ Troubleshooting

### Common Issues

#### 🔴 VRBOX won't connect
- Check if it's in Mouse mode (`@ + D`)
- Try ESP32-C3 reset
- Verify BLE name is exactly "VR BOX"

#### 🔴 PWM doesn't vary
- Check real VRBOX range in Serial Monitor
- Confirm `PWM_MIN/MAX` and `DEADBAND` values
- Test with oscilloscope on GPIO4/5 pins

#### 🔴 Unstable control
- Adjust `SMOOTHING_FACTOR` to higher value
- Check stable ESP32-C3 power supply
- Confirm GND connections

#### 🔴 Compilation errors
```bash
# Clean build cache
pio run --target clean

# Update libraries
pio lib update

# Check framework version
pio platform update espressif32
```

### Advanced Debug
```cpp
// Enable detailed debug (add at beginning)
#define DEBUG_VRBOX 1
#define DEBUG_PWM 1

// Check raw values
Serial.printf("Raw X: %d, Raw Y: %d\n", rawX, rawY);
```

## 📈 Performance

### Typical Benchmarks
- **BLE Latency**: 50-100ms
- **Update rate**: 10-20 Hz
- **PWM Resolution**: 12 bits (4096 levels)
- **Precision**: ±1μs in PWM generation
- **Consumption**: ~150mA (ESP32-C3 active + BLE)

### Limitations
- **Limited range**: VRBOX has smaller range than standard joysticks
- **BLE latency**: Inherent to Bluetooth protocol
- **Interference**: High 2.4GHz environments may affect performance

## 📚 Technical References

### VRBOX Protocol
- Based on HID over GATT (UUID 1812)
- 4-byte format for joystick/triggers
- 2-byte format for A/B/C/D buttons
- Mouse mode activated with `@ + D`

### Inspiration and Credits
- **BigJBehr**: [ESP32-Bluetooth-BLE-Remote-Control](https://github.com/BigJBehr/ESP32-Bluetooth-BLE-Remote-Control)
- **Bluetooth SIG**: HID Profile specifications
- **Espressif**: ESP32-C3 documentation

## 📄 License

This project is under MIT license. See `LICENSE` file for more details.

## 👨‍💻 Author

**Edilson Correa**
- GitHub: [@edilsoncorrea](https://github.com/edilsoncorrea)
- Project: VRBOX ESP32-C3 BLE Client

## 🤝 Contributions

Contributions are welcome! Please:
1. Fork the project
2. Create a branch for your feature
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## 📋 Changelog

### v2.1 (Current)
- ✅ Complete PWM system for RC control
- ✅ Optimized mapping for real VRBOX range
- ✅ Smoothing filters and deadband implemented
- ✅ Complete RC car control via GPIO4 (servo) and GPIO5 (ESC)
- ✅ Complete documentation updated in README.md
- ✅ Detailed debug and PWM monitoring

### v2.0 
- ✅ Stable BLE connection with VRBOX
- ✅ Complete parsing of HID protocols
- ✅ Detection of joystick, triggers and buttons

### v1.0
- ✅ Basic BLE proof of concept
- ✅ Initial connection with HID devices

---

## 🎯 Next Steps

- [ ] Implement automatic range calibration
- [ ] Add support for multiple VRBOXs
- [ ] Web interface for configuration
- [ ] Advanced telemetry and logging
- [ ] Support for other RC protocols (SBUS, PPM)

---

**⭐ If this project was useful, don't forget to give it a star!**
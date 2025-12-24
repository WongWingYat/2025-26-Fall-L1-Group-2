# Arduino Fall Detection and Alert System

A comprehensive fall detection and monitoring system using Arduino with accelerometer, motion sensor, and Bluetooth connectivity. The system detects falls, inactivity, and allows manual SOS triggering with multiple alert mechanisms.

## Table of Contents
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Bluetooth Commands](#bluetooth-commands)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [Future Improvements](#future-improvements)
- [License](#license)

## Features
- **Triple Alert System:**
  - Accelerometer-based fall detection
  - PIR motion sensor for inactivity monitoring
  - Manual SOS button for emergency situations
- **Multi-channel Notification:**
  - Visual RGB LED status indicators
  - Audible buzzer alerts
  - LCD display for real-time status
  - Bluetooth serial communication
- **Smart Alert Management:**
  - 30-second warning period before emergency notification
  - Automatic reset on motion detection recovery
  - Configurable thresholds and timing parameters
- **Bluetooth Remote Control:**
  - Real-time status monitoring
  - Remote command execution
  - Test and simulation functions

## Hardware Requirements

| Component | Quantity | Description |
|-----------|----------|-------------|
| Arduino Uno/Nano | 1 | Main microcontroller board |
| MMA8452Q Accelerometer | 1 | I2C 3-axis accelerometer |
| HC-05 Bluetooth Module | 1 | Serial Bluetooth communication |
| PIR Motion Sensor | 1 | HC-SR501 or similar |
| I2C LCD Display (16x2) | 1 | With I2C backpack |
| RGB LED (Common Cathode) | 1 | Or separate Red, Green, Blue LEDs |
| Buzzer (Active) | 1 | 5V active buzzer |
| Push Button | 1 | Tactile switch for SOS |
| Resistors | 3 | 220Ω for RGB LED |
| Jumper Wires | Several | Male-to-male and male-to-female |
| Breadboard | 1 | For prototyping |

## Wiring Diagram

### Pin Connections

| Component | Arduino Pin | Connection Notes |
|-----------|-------------|------------------|
| **HC-05 Bluetooth** | | |
| TX (HC-05) | D4 | → SoftwareSerial RX |
| RX (HC-05) | D5 | → SoftwareSerial TX |
| VCC | 5V | |
| GND | GND | |
| **MMA8452Q Accelerometer** | | |
| SDA | A4 | I2C Data |
| SCL | A5 | I2C Clock |
| VCC | 3.3V | **Use 3.3V only** |
| GND | GND | |
| **I2C LCD Display** | | |
| SDA | A4 | Shared with accelerometer |
| SCL | A5 | Shared with accelerometer |
| VCC | 5V | |
| GND | GND | |
| **PIR Motion Sensor** | | |
| OUT | D2 | Digital input |
| VCC | 5V | |
| GND | GND | |
| **RGB LED** | | |
| Red | D9 | PWM capable |
| Green | D10 | PWM capable |
| Blue | D11 | PWM capable |
| **Buzzer** | | |
| + | D8 | Digital output |
| - | GND | |
| **SOS Button** | | |
| One side | D3 | With internal pull-up |
| Other side | GND | |

### Schematic Overview
```text
            +-----------------------------------+
            |            Arduino Uno            |
            |                                   |
            |   +---------------------------+   |
            |   |         Power Bus         |   |
            |   | 5V  ====================  |   |
            |   | GND ====================  |   |
            |   +---------------------------+   |
            |                                   |

HC-05 Bluetooth | | MMA8452Q
+-----------+ | D4 RX ───────┐ A4 SDA ──────┐
| VCC 5V ├───┼───────────────────┼────────────────┤ 3.3V VCC |
| GND ├───┼───────────────────┼────────────────┤ GND |
| TX ├───┼→ D4 │ │ SDA A4 |
| RX ├───┼← D5 │ │ SCL A5 |
+-----------+ | D5 TX ───────┘ +----------+
| |
I2C LCD | | PIR Sensor
+-----------+ | A4 SDA ───────┐ +----------+
| VCC 5V ├───┼──────────────────┤ | VCC 5V │
| GND ├───┼──────────────────┤ | GND │
| SDA ├───┼──────────────────┘ | OUT D2│
| SCL ├───┼── A5 SCL +----------+
+-----------+ | |
| |
RGB LED | | Buzzer
Red D9 ────┤ | + D8│
Green D10 ───┤ | - GND│
Blue D11 ───┤ +----------+
| |
SOS Button | |
D3 ─────┤ |
GND ─────┤ |
+-----------------------------------+
```

## Installation

### 1. Software Requirements
- Arduino IDE 1.8.x or later
- Required Libraries (Install via Library Manager):
  - `Wire.h` (Built-in)
  - `LiquidCrystal_I2C.h` (by Frank de Brabander)
  - `SparkFun_MMA8452Q.h` (by SparkFun Electronics)
  - `SoftwareSerial.h` (Built-in)

### 2. Hardware Setup
1. Connect all components according to the wiring diagram
2. **Important**: The MMA8452Q accelerometer requires 3.3V, not 5V
3. Ensure proper pull-up resistors for I2C lines (usually on the modules)
4. Connect Arduino to computer via USB

### 3. Upload Code
1. Clone or download this repository
2. Open `fall_detection_system.ino` in Arduino IDE
3. Select correct board (Tools → Board → Arduino Uno)
4. Select correct port (Tools → Port)
5. Click Upload button

### 4. Bluetooth Pairing
1. Power on the system
2. On your smartphone/computer, scan for Bluetooth devices
3. Connect to "HC-05" (default password: "1234" or "0000")
4. Use a serial terminal app (like Serial Bluetooth Terminal) to communicate

## Configuration

### Key Constants in Code
```cpp
// Alert timing (in milliseconds)
const unsigned long INACTIVITY_WARNING = 60000;    // 1 minute
const unsigned long NOTIFICATION_DELAY = 30000;    // 30 seconds
const unsigned long PIR_CHECK_INTERVAL = 200;      // 200ms
const unsigned long ACCEL_CHECK_INTERVAL = 50;     // 50ms
const unsigned long DISPLAY_UPDATE = 2000;         // 2 seconds

// Fall detection sensitivity
const float FALL_THRESHOLD = 2.5;                  // g-force threshold

// Pin assignments
#define PIR_PIN 2
#define BUTTON_PIN 3
#define BUZZER_PIN 8
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11
```

### Adjusting Sensitivity

- **Fall Detection**: Modify `FALL_THRESHOLD` value (2.5g is typical for falls)
- **Inactivity Time**: Change `INACTIVITY_WARNING` (currently 60 seconds)
- **Alert Delay**: Adjust `NOTIFICATION_DELAY` (currently 30 seconds)

## Usage

### System States

#### Initialization (Blue LED)
- System booting up
- Waiting for Bluetooth connection

#### Monitoring Mode (Green LED)
- Bluetooth connected
- Actively monitoring sensors
- LCD shows inactivity timer

#### Alert Mode (Red LED + Buzzer)
- Fall, inactivity, or SOS detected
- 30-second countdown on LCD
- Multiple warning buzzer patterns

#### Warning Mode (Yellow LED)
- Bluetooth not connected
- System ready but not transmitting

### Normal Operation Flow

1. **Power on the system**
2. **Wait for Bluetooth connection** (LED turns green when connected)
3. **System continuously monitors:**
   - Accelerometer for sudden movements
   - PIR sensor for motion
   - Button for manual SOS
4. **When alert is triggered:**
   - Red LED lights up
   - Buzzer sounds pattern
   - LCD shows countdown
   - Bluetooth notification sent
5. **Alert can be cancelled by:**
   - Short button press (< 50ms)
   - Motion detection (for inactivity alerts)
   - Bluetooth `RESET` command

## Bluetooth Commands

Connect via serial Bluetooth terminal at 9600 baud:

| Command | Description | Response |
|---------|-------------|----------|
| `STATUS` | Get system status | Current state, inactivity time, connection status |
| `TEST` | System test | Verifies all components are working |
| `SOS` or `HELP` | Trigger manual alert | Immediate SOS alert activation |
| `RESET` | Cancel active alert | Resets system to monitoring mode |
| `SIMFALL` | Simulate fall | Tests fall detection without actual movement |

### Example Bluetooth Session
```text
System Connected - Single Trigger Mode
Commands: STATUS, TEST, SOS, RESET, SIMFALL

> STATUS
Status: Monitoring | Inactive: 45s | BT:Connected | Mode:Single Trigger

> TEST
System Test Normal
Accelerometer: Normal - Single Trigger Mode

> SIMFALL
⚠️ Fall Alert! Please confirm immediately!
```

## Troubleshooting

### Common Issues and Solutions

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| No Bluetooth Connection | Wrong baud rate | Ensure both ends use 9600 baud |
| | HC-05 not powered | Check 5V and GND connections |
| | Pairing issues | Default PIN is usually "1234" |
| Accelerometer Not Detected | Wrong voltage | MMA8452Q requires 3.3V, not 5V |
| | I2C address conflict | Check address (usually 0x1D) |
| | Wiring issues | Verify SDA (A4) and SCL (A5) |
| False Fall Detections | Too sensitive threshold | Increase FALL_THRESHOLD value |
| | Vibration interference | Isolate from vibrating surfaces |
| PIR Sensor Not Working | No motion in range | Adjust sensitivity and time delay pots |
| | Incorrect orientation | Point toward monitoring area |
| LCD Not Displaying | I2C address wrong | Common addresses: 0x27, 0x3F |
| | Contrast issue | Adjust potentiometer on I2C module |
| Buzzer Continuously On | Alert not reset | Press button or send RESET command |
| | Wiring short | Check BUZZER_PIN connection |

### Serial Monitor Debugging

Open Serial Monitor at 9600 baud to see debug information:

- System startup status
- Accelerometer readings
- Alert triggers
- Bluetooth commands
- System state changes

## Project Structure
```text
fall-detection-system/
├── fall_detection_system.ino    # Main Arduino sketch
├── README.md                    # This documentation
├── LICENSE                      # MIT License
├── schematics/
│   └── circuit_diagram.fzz      # Fritzing diagram (optional)
├── images/
│   ├── wiring_photo.jpg         # Actual wiring photo
│   └── enclosure_design.jpg     # 3D printed case (optional)
└── documentation/
    └── bluetooth_setup.pdf      # HC-05 pairing instructions
```

## Future Improvements

### Hardware Enhancements
- **Battery Power** - Add LiPo battery with charging circuit
- **GPS Module** - Include location in emergency notifications
- **WiFi/GSM Integration** - Add internet connectivity for remote alerts
- **Multiple Alert Outputs** - Add vibration motor, louder siren
- **Waterproof Enclosure** - For practical daily use

### Software Features
- **Data Logging** - SD card storage of events and sensor data
- **Machine Learning** - Improved fall detection algorithm
- **Multiple User Profiles** - Customizable thresholds per user
- **Mobile App** - Dedicated Android/iOS application
- **Cloud Integration** - Real-time monitoring dashboard

### Safety Features
- **Two-way Communication** - Voice communication through Bluetooth
- **Automatic Emergency Call** - Integration with emergency services
- **Medical Information** - Store and transmit medical details
- **Low Battery Warning** - Prevent unexpected shutdown
- **Self-test Routine** - Daily automatic system diagnostics

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments
- SparkFun Electronics for the MMA8452Q library
- Arduino community for extensive documentation and examples
- All contributors and testers who helped improve the system
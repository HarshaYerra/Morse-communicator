# ESP32 Morse Code Communication System

A real-time, two-way Morse code communication system using ESP32 boards with ESP-NOW protocol. Type messages in Morse code on one board and see them appear on another wirelessly!

## 🌟 Features

- **Real-time Morse Input**: Input dots and dashes using physical push buttons
- **Live Auto-decode**: Automatically converts Morse code to text after 500ms of inactivity
- **Wireless Communication**: Uses ESP-NOW for low-latency peer-to-peer messaging
- **16x2 LCD Display**: 
  - Line 1: Shows current Morse input (dots and dashes)
  - Line 2: Shows decoded text with blinking cursor
- **Full Duplex**: Both boards can send and receive messages simultaneously
- **Persistent Display**: Received messages stay on screen until you start typing
- **Intuitive Controls**: 5-button interface for complete message composition

## 🛠️ Hardware Requirements

### Per Board:
- 1x ESP32 Development Board
- 1x 16x2 I2C LCD Display (Address: 0x27)
- 5x Push Buttons (momentary, normally open)
- 5x 10kΩ Resistors (optional, ESP32 has internal pull-ups)
- Breadboard and jumper wires
- USB cable for programming

## 📌 Pin Configuration

| Component | GPIO Pin |
|-----------|----------|
| DOT Button | 14 |
| DASH Button | 27 |
| SPACE Button | 26 |
| SEND Button | 25 |
| BACKSPACE Button | 33 |
| LCD SDA | 21 |
| LCD SCL | 22 |

**Note**: All buttons use internal INPUT_PULLUP. Connect one terminal to GPIO and other to GND.

## 🔧 Wiring Diagram

```
ESP32          Component
-----          ---------
GPIO 14  ---→  DOT Button ---→ GND
GPIO 27  ---→  DASH Button ---→ GND
GPIO 26  ---→  SPACE Button ---→ GND
GPIO 25  ---→  SEND Button ---→ GND
GPIO 33  ---→  BACKSPACE Button ---→ GND
GPIO 21  ---→  LCD SDA
GPIO 22  ---→  LCD SCL
3.3V     ---→  LCD VCC
GND      ---→  LCD GND
```

## 📚 Software Dependencies

Install the following libraries via Arduino Library Manager:

1. **ESP32 Arduino Core** (v2.0.0 or higher)
2. **LiquidCrystal I2C** by Frank de Brabander

## 🚀 Installation & Setup

### 1. Find Your ESP32 MAC Addresses

Upload this sketch to each board to find its MAC address:

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

### 2. Update MAC Addresses in Code

- In **B1 code**: Set `peerAddress[]` to B2's MAC address
- In **B2 code**: Set `peerAddress[]` to B1's MAC address

Example:
```cpp
// In B1 - set to B2's MAC
uint8_t peerAddress[] = {0x78, 0x1C, 0x3C, 0xB9, 0x85, 0x20};

// In B2 - set to B1's MAC
uint8_t peerAddress[] = {0x78, 0x1C, 0x3C, 0xB7, 0xB9, 0x24};
```

### 3. Upload Code

- Upload `morse_B1.ino` to Board 1
- Upload `morse_B2.ino` to Board 2

### 4. Power On and Test!

Both boards should display "Morse Communicator" followed by "B1/B2 Ready..."

## 🎮 How to Use

### Button Functions:

| Button | Function |
|--------|----------|
| **DOT** | Add a dot (.) to current Morse sequence |
| **DASH** | Add a dash (-) to current Morse sequence |
| **SPACE** | Decode current Morse & add space to text |
| **BACKSPACE** | Remove last Morse element or text character |
| **SEND** | Send complete message to peer board |

### Typing Process:

1. **Enter Morse Code**: Press DOT/DASH buttons to build Morse sequence
   - Example: `.- -` appears on Line 1
2. **Auto-decode**: After 500ms of no input, Morse converts to character
   - Line 1 clears, Line 2 shows: `AB_`
3. **Add Spaces**: Press SPACE button to add space between words
4. **Send Message**: Press SEND to transmit to peer board
5. **Receive Messages**: Peer board shows "Received:" with your message

### Morse Code Reference:

```
A .-    B -...  C -.-.  D -..   E .     F ..-.
G --.   H ....  I ..    J .---  K -.-   L .-..
M --    N -.    O ---   P .--.  Q --.-  R .-.
S ...   T -     U ..-   V ...-  W .--   X -..-
Y -.--  Z --..

0 -----  1 .----  2 ..---  3 ...--  4 ....-
5 .....  6 -....  7 --...  8 ---..  9 ----.

. .-.-.-  , --..--  ? ..--..  ! -.-.--
```

## 🎯 Features Explained

### Auto-decode Timeout
- Morse characters are automatically decoded after 500ms of inactivity
- No need to press SPACE after every character
- SPACE button adds actual space between words

### Smart Backspace
- If building Morse: Removes last dot/dash
- If Morse is empty: Removes last decoded character

### Persistent Received Messages
- Received messages stay on screen indefinitely
- Automatically clears when you start typing new message
- No timeout or manual dismiss needed

### Debounced Button Input
- 50ms hardware debouncing per button
- Single-press registration prevents double inputs
- Reliable even with mechanical switch bounce

## 🔍 Troubleshooting

### LCD shows garbage characters
- Check I2C address (use I2C scanner sketch)
- Verify SDA/SCL connections
- Try different I2C LCD library

### Buttons not responding
- Verify GPIO pin connections
- Check button wiring (button → GPIO → GND)
- Ensure buttons are normally-open (momentary)

### Messages not sending
- Verify MAC addresses are correctly swapped between boards
- Check Serial Monitor for ESP-NOW errors
- Ensure both boards are powered on

### ESP-NOW init failed
- Update ESP32 Arduino Core to latest version
- Check WiFi mode is set to WIFI_STA
- Try re-uploading the sketch

## 📊 Technical Specifications

- **Communication Protocol**: ESP-NOW (2.4GHz)
- **Range**: Up to 200m (line of sight)
- **Latency**: <10ms typical
- **Max Message Length**: 127 characters
- **Display Update**: Real-time, no flicker
- **Power Consumption**: ~80mA per board (active)

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is open source and available under the MIT License.

## 👨‍💻 Author

Created for learning ESP32, ESP-NOW, and Morse code communication.

## 🙏 Acknowledgments

- ESP32 Community for ESP-NOW examples
- Arduino Community for LCD libraries
- International Morse Code standard

---

**Star ⭐ this repo if you found it helpful!**

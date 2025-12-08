# ESP32 MQTT LED Control System

A complete IoT system for remotely controlling ESP32-based LED matrix displays through a modern web interface using MQTT communication.

## Features

- **Time and Weather Updates**  
  Displays accurate local time and live weather information directly on the LED display.

- **Real-time Message Display**  
  Allows sending custom messages to any connected device with immediate on-screen updates.

- **LED Brightness Control**  
  Adjustable brightness levels (0–15) via a simple web interface.

- **Device Management Dashboard**  
  View and manage multiple ESP32 devices with clear online/offline indicators.

- **Real-time System Updates**  
  WebSocket and MQTT integration ensures instant communication and device status synchronization.

- **Flexible Messaging Options**  
  Supports broadcasting messages to all devices or targeting individual devices.

- **Automatic Device Status Detection**  
  Devices are monitored continuously, and offline states are detected within a 15-second timeout.

## Architecture

```
┌─────────────────┐     MQTT       ┌──────────────┐
│  Web Interface  │◄──────────────►│ HiveMQ Broker│
│  (localhost)    │                │   (Cloud)    │
└─────────────────┘                └──────────────┘
         ▲                                ▲
         │ WebSocket                      │ MQTT
         │                                │
         ▼                                ▼
  ┌──────────────┐                ┌──────────────┐
  │ Node.js      │                │  ESP32 #1    │
  │ Server       │                │  + MAX7219   │
  └──────────────┘                └──────────────┘
                                          │
                                          ▼
                                  ┌──────────────┐
                                  │  ESP32 #2    │
                                  │  + MAX7219   │
                                  └──────────────┘
```


## Quick Start

### Prerequisites

- **ESP-IDF** v5.0+ installed and configured
- **Node.js** v14+ and npm
- ESP32 development board
- MAX7219 LED matrix display (4 modules)
- WiFi network

### 1. Setup Web Application

```bash
cd webapp
npm install
npm start
```

The web interface will be available at **http://localhost:3000**

### 2. Flash ESP32 Firmware

Update WiFi credentials in `main/wifi_sta/wifi_sta.h`:

```c
#define WIFI_SSID      "YourWiFiName"
#define WIFI_PASS      "YourWiFiPassword"
```

Build and flash:

```bash
idf.py build
idf.py flash monitor
```

### 3. Connect Hardware

| MAX7219 Pin | ESP32 Pin |
| ----------- | --------- |
| VCC         | 3.3V      |
| GND         | GND       |
| DIN (MOSI)  | GPIO 11   |
| CLK (SCK)   | GPIO 12   |
| CS          | GPIO 10   |

<!-- ## MQTT Topics

| Topic                             | Direction   | Description               |
| --------------------------------- | ----------- | ------------------------- |
| `classplate/message`              | Web → ESP32 | Broadcast message         |
| `classplate/message/{deviceId}`   | Web → ESP32 | Device-specific message   |
| `classplate/intensity`            | Web → ESP32 | Broadcast intensity       |
| `classplate/intensity/{deviceId}` | Web → ESP32 | Device-specific intensity |
| `classplate/status/{deviceId}`    | ESP32 → Web | Device status (JSON)      |
| `classplate/heartbeat/{deviceId}` | ESP32 → Web | Heartbeat (every 10s)     |
 -->

## License

This project is licensed under the MIT License.  
See the [LICENSE](./LICENSE) file for details.

## Support My Work

If you find my work useful, consider supporting me by buying me a coffee!

<a href="https://buymeacoffee.com/mohitdeoli" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

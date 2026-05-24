---
publishDate: 2026-05-24T00:00:00Z
title: MYOSA SwarmSense Lite – IoT-Based Collaborative Survivor Detection System
excerpt: An IoT-based disaster rescue and human presence detection system using ESP32 and MYOSA SwarmSense Lite sensors.
image: myosa-swarmsense-lite/rescue-detector-cover.jpg
tags:
  - IoT
  - ESP32
  - Disaster Rescue
  - Sensors
  - Blynk
  - Human-Presence
  - MYOSA
  - SwarmSense
---

> Intelligent sensing for faster and safer disaster rescue operations.
---
## Acknowledgements

We sincerely thank the MYOSA community, IEEE Sensors Council, mentors, and faculty members for their valuable guidance and support throughout the development of this project.
---

## Overview

MYOSA SwarmSense Lite is an IoT-based disaster rescue system designed to detect human presence in emergency situations.  
The project uses ESP32 along with multiple sensors to monitor environmental conditions and provide alerts through OLED display, buzzer, LEDs, and Blynk IoT platform.

**Key Features:**

* Human presence detection
* OLED live status monitoring
* Blynk mobile monitoring
* Alert system using buzzer and LEDs
* Multi-sensor environmental monitoring

---

## Demo / Examples

### Images

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-cover.jpg" width="800"><br/>
  <i>Project Cover Image</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-oled-env.jpg" width="800"><br/>
  <i>OLED Environment Monitoring</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-oled-motion.jpg" width="800"><br/>
  <i>Motion Detection Display</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-oled-alert.jpg" width="800"><br/>
  <i>Alert Status on OLED</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-oled-clear.jpg" width="800"><br/>
  <i>Safe Status Display</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-blynk.jpg" width="800"><br/>
  <i>Blynk IoT Monitoring</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-arduino.jpg" width="800"><br/>
  <i>Arduino IDE Code</i>
</p>

<p align="center">
  <img src="/assets/images/myosa-swarmsense-lite/rescue-detector-hardware.jpg" width="800"><br/>
  <i>Hardware Setup</i>
</p>

### **Videos**

[Presentation Video] (https://github.com/user-attachments/assets/b3164367-fa68-44b0-b787-afb7ef06aa84)


<video controls width="100%">
  <source src="/presentation.mp4" type="video/mp4">
</video>


[Demonstration Video] (https://github.com/user-attachments/assets/e29214b2-8be0-471c-a1de-5870f047a968)



<video controls width="100%">
  <source src="/demonstration.mp4" type="video/mp4">
</video>


---
## Features (Detailed)

### 1. Environmental Monitoring

The system continuously monitors environmental conditions using multiple sensors connected to ESP32.

### 2. Human Presence Detection

Motion and environmental data are analyzed to identify possible human presence during disaster situations.

### 3. OLED Status Display

Real-time sensor values and alerts are displayed on the OLED screen.

### 4. IoT Monitoring using Blynk

Sensor data can be monitored remotely through the Blynk mobile application.

### 5. Alert System

When abnormal conditions are detected, LEDs glow and buzzer alerts are activated.

---

## Usage Instructions

1. Connect all sensors properly to ESP32.
2. Upload the Arduino code using Arduino IDE.
3. Configure WiFi and Blynk credentials.
4. Power the system.
5. Monitor readings through OLED and Blynk app.

---

## Tech Stack

* ESP32
* Arduino IDE
* Embedded C++
* Blynk IoT Platform
* OLED Display (SSD1306)
* WiFi Module
* MQ Gas Sensor
* Sound Sensor
* Motion / Presence Detection Sensors
* IoT Monitoring System

---

## Requirements / Installation

Hardware Required:

- ESP32 Dev Module
- DHT11 Temperature and Humidity Sensor
- MQ-135 Gas Sensor Module
- KY-037 Sound Sensor Module
- MPU6050 Accelerometer and Gyroscope (MYOSA SwarmSense Lite)
- APDS9960 Gesture and Proximity Sensor (MYOSA SwarmSense Lite)
- SSD1306 OLED Display 0.96 inch I2C (MYOSA SwarmSense Lite)
- Active Buzzer Module
- LED with 220 ohm Resistor
- Breadboard and Jumper Wires
- USB Power Bank or 5V USB Power Supply


Arduino Libraries — Install via Arduino IDE Library Manager:

- Blynk by Volodymyr Shymanskyy
- DHT sensor library by Adafruit
- Adafruit MPU6050
- Adafruit APDS9960
- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit Unified Sensor

Blynk IoT Setup:

1. Create account at console.blynk.cloud
2. Create new Template
   Name: HumanPresenceDetector
   Hardware: ESP32
   Connection: WiFi
3. Add Datastreams V0 to V10 (Virtual Pins)
4. Create Event named: human_detected
   Enable push notification
5. Create new Device and copy Auth Token
6. In Arduino code replace:
   BLYNK_TEMPLATE_ID with your Template ID
   BLYNK_AUTH_TOKEN with your Auth Token
   ssid with your WiFi name
   password with your WiFi password
7. Upload code and verify on Serial Monitor

## Code & Technical Content

### Arduino Source Code

```cpp
Blynk.virtualWrite(V0, temperature);

if (gasAnalog > GAS_ALERT_LEVEL) {
  digitalWrite(BUZZER_PIN, HIGH);
}
```

Full Arduino source code:

[Open Full Code](code/swarmsense-lite.ino)

## Contribution Notes

This project was developed as part of MYOSA 5.0 competition. If you would like to build upon this project, you are welcome to fork this repository and submit improvements. Suggested areas for contribution include adding GPS location tracking, LoRa long-range communication, waterproof enclosure design, and AI-based pattern recognition for improved detection accuracy.
   

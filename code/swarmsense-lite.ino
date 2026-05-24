/*
 * ============================================================
 *   HUMAN PRESENCE DETECTOR — DISASTER RESCUE SYSTEM
 *   ESP32 + Blynk IoT + Multiple Sensors
 * ============================================================
 *
 *  SENSORS & PINS:
 *   DHT11        → GPIO15  (Temp + Humidity)
 *   MQ-135 AOUT  → GPIO34  (Analog gas level)
 *   MQ-135 DOUT  → GPIO16  (Digital threshold)
 *   KY-037 AOUT  → GPIO32  (Analog sound level)
 *   KY-037 DOUT  → GPIO26  (Digital sound detect)
 *   MPU6050      → GPIO21(SDA), GPIO22(SCL)
 *   OLED         → GPIO21(SDA), GPIO22(SCL)
 *   APDS9960     → GPIO21(SDA), GPIO22(SCL)
 *   APDS9960 INT → GPIO13
 *   LED          → GPIO2  (via 220Ω resistor)
 *   BUZZER       → GPIO5
 *
 *  LIBRARIES TO INSTALL (Arduino Library Manager):
 *   - Blynk (by Volodymyr Shymanskyy) v1.3.2+
 *   - DHT sensor library (by Adafruit)
 *   - Adafruit MPU6050
 *   - Adafruit APDS9960
 *   - Adafruit SSD1306
 *   - Adafruit GFX Library
 *   - Adafruit Unified Sensor
 *   - Wire (built-in)
 *
 *  BLYNK VIRTUAL PINS:
 *   V0  → Temperature (°C)
 *   V1  → Humidity (%)
 *   V2  → Gas Level (analog 0-4095)
 *   V3  → Sound Level (analog 0-4095)
 *   V4  → Accel X
 *   V5  → Accel Y
 *   V6  → Accel Z
 *   V7  → Gesture direction (text)
 *   V8  → Vibration magnitude
 *   V9  → Alert status (text)
 *
 * ============================================================
 */

// ─── BLYNK CONFIG (fill in your details) ────────────────────
#define BLYNK_TEMPLATE_ID "TMPL3YnTtiANz"
#define BLYNK_TEMPLATE_NAME "HumanPresenceDetector"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// Silence Blynk serial prints if needed
#define BLYNK_PRINT Serial

// ─── LIBRARIES ──────────────────────────────────────────────
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ─── WIFI CREDENTIALS ────────────────────────────────────────
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ─── PIN DEFINITIONS ─────────────────────────────────────────
#define DHT_PIN         17
#define DHT_TYPE        DHT11

#define MQ135_AOUT_PIN  34
#define MQ135_DOUT_PIN  16

#define KY037_AOUT_PIN  32
#define KY037_DOUT_PIN  26

#define APDS_INT_PIN    13

#define LED_PIN         2
#define BUZZER_PIN      16

// ─── OLED DISPLAY CONFIG ─────────────────────────────────────
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// ─── ALERT THRESHOLDS ────────────────────────────────────────
#define TEMP_ALERT_HIGH     30.0   
#define HUMIDITY_ALERT_HIGH 80.0   
#define GAS_ALERT_LEVEL     700
#define SOUND_ALERT_LEVEL   1500
#define VIBRATION_THRESHOLD 15.0
#define TILT_THRESHOLD      50.0

// ─── OBJECT DECLARATIONS ─────────────────────────────────────
DHT            dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050  mpu;
Adafruit_APDS9960 apds;
Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BlynkTimer     timer;

// ─── GLOBAL STATE ────────────────────────────────────────────
float    temperature      = 0;
float    humidity         = 0;
int      gasAnalog        = 0;
bool     gasDigital       = false;
int      soundAnalog      = 0;
bool     soundDigital     = false;
float    accelX           = 0, accelY = 0, accelZ = 0;
float    gyroX            = 0, gyroY = 0, gyroZ = 0;
float    vibrationMag     = 0;
float    prevAccelMag     = 9.8;
String   gestureText      = "None";
String   alertStatus      = "ALL CLEAR";
bool     humanDetected    = false;

// Buzzer control
bool     buzzerActive     = false;
unsigned long buzzerStart = 0;
#define BUZZER_DURATION   500

// Blynk notification cooldown
unsigned long lastNotify  = 0;
#define NOTIFY_COOLDOWN   10000

// OLED page cycling
int      oledPage         = 0;
unsigned long lastPageSwitch = 0;
#define PAGE_INTERVAL     3000

// Global variables
float offsetX = 0, offsetY = 0, offsetZ = 0;

void calibrateMPU() {
  Serial.println("Calibrating MPU6050...");
  float sx=0, sy=0, sz=0;
  for(int i=0; i<100; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    sx += a.acceleration.x;
    sy += a.acceleration.y;
    sz += a.acceleration.z;
    delay(10);
  }
  offsetX = sx/100;
  offsetY = sy/100;
  offsetZ = (sz/100) - 9.8;
  Serial.println("MPU Calibration Done!");
}

// ─── SETUP ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(MQ135_DOUT_PIN, INPUT);
  pinMode(KY037_DOUT_PIN, INPUT);
  pinMode(APDS_INT_PIN,   INPUT_PULLUP);
  pinMode(LED_PIN,        OUTPUT);
  pinMode(BUZZER_PIN,     OUTPUT);

  digitalWrite(LED_PIN,    LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("\n====================================");
  Serial.println(" HUMAN PRESENCE DETECTOR BOOTING");
  Serial.println("====================================");

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("ERROR: OLED not found! Check wiring.");
  } else {
    Serial.println("OK: OLED ready");
    bootScreen();
  }

  dht.begin();
  Serial.println("OK: DHT11 ready");

  if (!mpu.begin(0X69)) {
    Serial.println("ERROR: MPU6050 not found! Check wiring.");
    oledError("MPU6050 FAIL");
  } else {
    Serial.println("OK: MPU6050 ready");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    calibrateMPU();
  }

  if (!apds.begin()) {
    Serial.println("ERROR: APDS9960 not found! Check wiring.");
    oledError("APDS9960 FAIL");
  } else {
    Serial.println("OK: APDS9960 ready");
    apds.enableProximity(true);
    apds.enableGesture(true);
    apds.enableColor(true);
  }

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  showOLED_Connecting();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("OK: Blynk connected");

  timer.setInterval(3000L, readAllSensors);
  timer.setInterval(500L,  updateOLED);
  timer.setInterval(3000L, sendToBlynk);
  timer.setInterval(100L,  manageBuzzer);

  Serial.println("====================================");
  Serial.println(" SYSTEM READY — Monitoring...");
  Serial.println("====================================\n");
}

// ─── MAIN LOOP ────────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
  checkGesture();
}

// ═══════════════════════════════════════════════════════════════
//   SENSOR READING FUNCTIONS
// ═══════════════════════════════════════════════════════════════

void readAllSensors() {
  readDHT11();
  readMQ135();
  readKY037();
  readMPU6050();
  evaluateAlerts();
}

void readDHT11() {
  for(int i = 0; i < 3; i++) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h) && t > 0 && h > 0) {
      temperature = t;
      humidity    = h;
      Serial.printf("[DHT11] Temp: %.1f°C  Hum: %.1f%%\n", t, h);
      return;
    }
    delay(500);
  }
  Serial.println("[DHT11] ERROR - Check wiring & resistor!");
}

void readMQ135() {
  gasAnalog  = analogRead(MQ135_AOUT_PIN);
  gasDigital = digitalRead(MQ135_DOUT_PIN);
  Serial.printf("[MQ135] Analog: %d  Digital: %s\n",
                gasAnalog, gasDigital ? "ALERT" : "OK");
}

void readKY037() {
  soundAnalog  = analogRead(KY037_AOUT_PIN);
  soundDigital = digitalRead(KY037_DOUT_PIN);
  Serial.printf("[KY037] Analog: %d  Detected: %s\n",
                soundAnalog, soundDigital ? "YES" : "No");
}

void readMPU6050() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  accelX = a.acceleration.x - offsetX;
  accelY = a.acceleration.y - offsetY;
  accelZ = a.acceleration.z - offsetZ;
  gyroX  = g.gyro.x;
  gyroY  = g.gyro.y;
  gyroZ  = g.gyro.z;

  float currentMag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  vibrationMag = abs(currentMag - prevAccelMag);
  prevAccelMag = currentMag;

  Serial.printf("[MPU6050] Ax:%.2f Ay:%.2f Az:%.2f | Vib:%.3f\n",
                accelX, accelY, accelZ, vibrationMag);
}

void checkGesture() {
  uint8_t gesture = apds.readGesture();
  switch (gesture) {
    case APDS9960_UP:    gestureText = "UP";    Serial.println("[APDS] Gesture: UP");    break;
    case APDS9960_DOWN:  gestureText = "DOWN";  Serial.println("[APDS] Gesture: DOWN");  break;
    case APDS9960_LEFT:  gestureText = "LEFT";  Serial.println("[APDS] Gesture: LEFT");  break;
    case APDS9960_RIGHT: gestureText = "RIGHT"; Serial.println("[APDS] Gesture: RIGHT"); break;
    default: break;
  }
}

// ═══════════════════════════════════════════════════════════════
//   ALERT LOGIC
// ═══════════════════════════════════════════════════════════════

void evaluateAlerts() {
  bool anyAlert     = false;
  bool triggerBuzz  = false;
  bool triggerLED   = false;
  String alerts     = "";

  if (gasAnalog > GAS_ALERT_LEVEL || gasDigital == HIGH) {
    triggerBuzz = true;
    anyAlert    = true;
    alerts     += "GAS! ";
    Serial.println(">>> ALERT: High Gas Level detected!");
  }

  if (soundDigital == HIGH || soundAnalog > SOUND_ALERT_LEVEL) {
    triggerBuzz = true;
    anyAlert    = true;
    alerts     += "SOUND! ";
    Serial.println(">>> ALERT: Sound detected!");
  }

  if (temperature > TEMP_ALERT_HIGH) {
    triggerLED = true;
    anyAlert   = true;
    alerts    += "TEMP! ";
    Serial.printf(">>> ALERT: High Temperature: %.1f°C\n", temperature);
  }

  if (humidity > HUMIDITY_ALERT_HIGH) {
    triggerLED = true;
    anyAlert   = true;
    alerts    += "HUMID! ";
    Serial.printf(">>> ALERT: High Humidity: %.1f%%\n", humidity);
  }

  if (vibrationMag > VIBRATION_THRESHOLD) {
    triggerBuzz = true;
    anyAlert    = true;
    alerts     += "VIB! ";
    Serial.printf(">>> ALERT: Vibration detected: %.3f\n", vibrationMag);
  }

  if (abs(accelX) > TILT_THRESHOLD || abs(accelY) > TILT_THRESHOLD) {
    triggerBuzz = true;
    anyAlert    = true;
    alerts     += "TILT! ";
    Serial.printf(">>> ALERT: Tilt detected! X:%.2f Y:%.2f\n", accelX, accelY);
  }

  if (triggerLED) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    if (anyAlert) {
      blinkLED(2);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }

  if (triggerBuzz && !buzzerActive) {
    startBuzzer();
  }

  if (anyAlert) {
    humanDetected = true;
    alertStatus   = "ALERT: " + alerts;
  } else {
    humanDetected = false;
    alertStatus   = "ALL CLEAR";
    digitalWrite(LED_PIN, LOW);
  }

  if (anyAlert && (millis() - lastNotify > NOTIFY_COOLDOWN)) {
    Blynk.logEvent("human_detected", "ALERT! " + alerts);
    lastNotify = millis();
    Serial.println(">>> Blynk notification sent!");
  }
}

// ═══════════════════════════════════════════════════════════════
//   BUZZER CONTROL
// ═══════════════════════════════════════════════════════════════

void startBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerActive = true;
  buzzerStart  = millis();
  Serial.println("[BUZZER] ON");
}

void manageBuzzer() {
  if (buzzerActive && (millis() - buzzerStart >= BUZZER_DURATION)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    Serial.println("[BUZZER] OFF");
  }
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}

// ═══════════════════════════════════════════════════════════════
//   BLYNK DATA PUSH
// ═══════════════════════════════════════════════════════════════

void sendToBlynk() {
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, gasAnalog);
  Blynk.virtualWrite(V3, soundAnalog);
  Blynk.virtualWrite(V4, accelX);
  Blynk.virtualWrite(V5, accelY);
  Blynk.virtualWrite(V6, accelZ);
  Blynk.virtualWrite(V7, gestureText);
  Blynk.virtualWrite(V8, vibrationMag);
  Blynk.virtualWrite(V9, alertStatus);

  gestureText = "None";
}

BLYNK_WRITE(V10) {
  int val = param.asInt();
  if (val == 1) {
    alertStatus   = "RESET by App";
    humanDetected = false;
    digitalWrite(LED_PIN,    LOW);
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    Serial.println("[Blynk] Manual reset from app.");
  }
}

// ═══════════════════════════════════════════════════════════════
//   OLED DISPLAY
// ═══════════════════════════════════════════════════════════════

void updateOLED() {
  if (millis() - lastPageSwitch > PAGE_INTERVAL) {
    oledPage = (oledPage + 1) % 3;
    lastPageSwitch = millis();
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("== RESCUE DETECTOR ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  if (oledPage == 0) {
    display.setCursor(0, 12);
    display.printf("T: %.1f C  H: %.0f%%", temperature, humidity);
    display.setCursor(0, 22);
    display.printf("Gas: %d", gasAnalog);
    display.setCursor(70, 22);
    display.print(gasAnalog > GAS_ALERT_LEVEL ? "!HIGH!" : "OK");
    display.setCursor(0, 32);
    display.printf("Sound: %d", soundAnalog);
    display.setCursor(70, 32);
    display.print(soundDigital ? "DETECT!" : "---");
    display.setCursor(0, 42);
    display.printf("Humidity: %.0f%%", humidity);
    display.setCursor(0, 54);
    display.setTextSize(1);
    display.print("Pg 1/3 ENV");

  } else if (oledPage == 1) {
    display.setCursor(0, 12);
    display.printf("X:%.2f Y:%.2f", accelX, accelY);
    display.setCursor(0, 22);
    display.printf("Z:%.2f", accelZ);
    display.setCursor(0, 32);
    display.printf("Vib: %.3f", vibrationMag);
    display.setCursor(0, 42);
    display.print("Gesture: ");
    display.print(gestureText);
    display.setCursor(0, 54);
    display.print("Pg 2/3 MOTION");

  } else if (oledPage == 2) {
    display.setCursor(0, 12);
    if (humanDetected) {
      display.setTextSize(1);
      display.print("!! HUMAN DETECTED !!");
      display.setCursor(0, 24);
      display.setTextSize(1);
      display.print(alertStatus.substring(0, 21));
    } else {
      display.setTextSize(1);
      display.setCursor(10, 18);
      display.print("* ALL CLEAR *");
      display.setCursor(0, 30);
      display.print("No human presence");
    }
    display.setCursor(0, 42);
    display.printf("Temp alert: >%.0fC", TEMP_ALERT_HIGH);
    display.setCursor(0, 54);
    display.print("Pg 3/3 ALERTS");
  }

  display.display();
}

void bootScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 5);
  display.print("RESCUE DETECTOR");
  display.setTextSize(1);
  display.setCursor(5, 18);
  display.print("Human Presence v1.0");
  display.setCursor(15, 30);
  display.print("Initializing...");
  display.setCursor(5, 45);
  display.print("ESP32 + Blynk IoT");
  display.display();
  delay(2500);
}

void oledError(String msg) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("!! ERROR !!");
  display.setCursor(0, 25);
  display.print(msg);
  display.display();
  delay(2000);
}

void showOLED_Connecting() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.print("Connecting WiFi...");
  display.setCursor(5, 25);
  display.print(ssid);
  display.setCursor(5, 40);
  display.print("Please wait...");
  display.display();
}

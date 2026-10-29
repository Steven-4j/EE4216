/*
 * EE4216 Lab 3 - FINAL Sensor Node A (Tasks 1, 2, 4)
 * ESP32-S3, Arduino IDE, Serial Monitor 115200
 * 1) STA joins Gateway B's AP
 * 2) HC-SR04 distance -> HTTP POST /api/sensor every ~5 sec
 * 4) HTTP Server POST /api/led controls one selected NeoPixel
 *
 * Install ArduinoJson and Adafruit NeoPixel libraries.
 * HC-SR04 Echo is 5V: use the resistor divider from Lab 1!
 */
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// ===== SETTINGS =====
const char* GATEWAY_AP_SSID = "A0309529W";
const char* GATEWAY_AP_PASSWORD = "YOUR_GATEWAY_AP_PASSWORD";
const int TRIG_PIN = 4;
const int ECHO_PIN = 5; // via voltage divider
const int LED_PIN = 7;  // IMPORTANT: change to actual LED strip DIN GPIO
const int NUM_LEDS = 8;
// ====================

const char* SENSOR_POST_URL = "http://192.168.4.1/api/sensor";
const unsigned long SEND_INTERVAL_MS = 5000UL;
const unsigned long WIFI_RETRY_INTERVAL_MS = 10000UL;
unsigned long lastSendTime = 0;
unsigned long lastWiFiRetry = 0;
bool ipPrinted = false;
long lastAppliedEntryId = -1; // Avoid applying the same cloud command twice

WebServer server(80);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#if ARDUINOJSON_VERSION_MAJOR >= 7
  #define NEW_JSON(name) JsonDocument name
#else
  #define NEW_JSON(name) StaticJsonDocument<256> name
#endif

bool readRgbByte(JsonVariantConst v, int &out) {
  if (!(v.is<int>() || v.is<long>())) return false;
  long n = v.as<long>();
  if (n < 0 || n > 255) return false;
  out = (int)n;
  return true;
}

// TASK 4: Gateway sends JSON {index,r,g,b,entry_id}.
void handleLed() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
    return;
  }
  NEW_JSON(doc);
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
    return;
  }
  JsonVariantConst vi = doc["index"];
  if (!(vi.is<int>() || vi.is<long>()) || vi.as<long>() < 0 || vi.as<long>() >= NUM_LEDS) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid LED index\"}");
    return;
  }
  int r, g, b;
  if (!readRgbByte(doc["r"], r) || !readRgbByte(doc["g"], g) || !readRgbByte(doc["b"], b)) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid RGB\"}");
    return;
  }
  long entryID = -1;
  JsonVariantConst commandId = doc["entry_id"];
  if (!commandId.isNull()) {
    if (!(commandId.is<int>() || commandId.is<long>()) || commandId.as<long>() <= 0) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid entry_id\"}");
      return;
    }
    entryID = commandId.as<long>();
  }

  int index = vi.as<int>();
  bool repeated = (entryID > 0 && entryID == lastAppliedEntryId);
  if (!repeated) {
    strip.setPixelColor(index, strip.Color(r, g, b));
    strip.show(); // Only this pixel changes; other LEDs retain their colors.
    if (entryID > 0) lastAppliedEntryId = entryID;
    Serial.printf("[CHECK-OFF 4] LED %d -> RGB(%d,%d,%d); entry=%ld\n",
                  index, r, g, b, entryID);
  }
  NEW_JSON(ack);
  ack["ok"] = true;
  ack["index"] = index;
  ack["entry_id"] = entryID;
  ack["duplicate"] = repeated;
  String payload;
  serializeJson(ack, payload);
  server.send(200, "application/json", payload);
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return NAN;
  float distance = duration * 0.0343f / 2.0f;
  if (!isfinite(distance) || distance <= 0 || distance > 400) return NAN;
  return distance;
}

// TASK 2: Upload sensor reading to Gateway AP IP (not to ThingSpeak directly).
void sendSensorReading() {
  if (WiFi.status() != WL_CONNECTED) return;
  float distance = measureDistance();
  if (!isfinite(distance)) {
    Serial.println("[TASK 2] Invalid ultrasonic reading, skipped");
    return;
  }
  NEW_JSON(doc);
  doc["node"] = "sensor01";
  doc["distance"] = roundf(distance * 100.0f) / 100.0f;
  String body;
  serializeJson(doc, body);

  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(2500);
  http.setTimeout(2500);
  if (!http.begin(client, SENSOR_POST_URL)) {
    Serial.println("[TASK 2] HTTP begin failed");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  int status = http.POST(body);
  if (status > 0) {
    Serial.printf("[TASK 2] Distance %.2f cm -> HTTP %d\n", distance, status);
  } else {
    Serial.printf("[TASK 2] HTTP error: %s\n", http.errorToString(status).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  strip.begin();
  strip.clear();
  strip.show();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(GATEWAY_AP_SSID, GATEWAY_AP_PASSWORD);
  server.on("/api/led", HTTP_POST, handleLed);
  server.begin();
  lastSendTime = millis() - SEND_INTERVAL_MS;
  lastWiFiRetry = millis();
  Serial.println("[TASK 1] Connecting to Gateway AP...");
}

void loop() {
  server.handleClient();  // Receive LED commands from Gateway B
  unsigned long now = millis();
  if (WiFi.status() == WL_CONNECTED) {
    if (!ipPrinted) {
      Serial.print("[CHECK-OFF 1] Sensor DHCP IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("[TASK 1] Gateway AP IP: ");
      Serial.println(WiFi.gatewayIP());
      ipPrinted = true;
    }
  } else {
    ipPrinted = false;
    if (now - lastWiFiRetry >= WIFI_RETRY_INTERVAL_MS) {
      lastWiFiRetry = now;
      WiFi.reconnect();
      Serial.println("[Wi-Fi] Reconnecting to Gateway AP...");
    }
  }
  if (now - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = now;
    sendSensorReading();
  }
  delay(2);
}

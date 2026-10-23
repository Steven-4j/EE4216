#include <WiFi.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <ArduinoJson.h>
#include <NewPing.h>
#include <Adafruit_NeoPixel.h>
#include "lab_page.h"

// ---------- Network ----------
const bool  USE_AP    = false;              
const char* AP_SSID   = "A0305871A";
const char* AP_PASS   = "ee4216lab2";
const char* STA_SSID  = "TP-Link_6FCB";
const char* STA_PASS  = "72972786";
const uint32_t STA_TIMEOUT_MS = 15000;

// ---------- Hardware ----------
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int LED_PIN  = 6;
const int NUM_LEDS = 8;
const int MAX_DISTANCE_CM = 200;

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE_CM);
Adafruit_NeoPixel bar(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// Eight independent logical LED states
struct LedState { uint8_t r, g, b, brightness; };
LedState leds[NUM_LEDS];

// ---------- Helpers ----------
void sendJson(int code, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}

void sendError(int code, const String& msg) {
  JsonDocument doc;
  doc["error"] = msg;
  String out;
  serializeJson(doc, out);
  sendJson(code, out);
}

void ledToJson(JsonObject o, int id) {
  o["id"] = id;
  o["r"] = leds[id].r;
  o["g"] = leds[id].g;
  o["b"] = leds[id].b;
  o["brightness"] = leds[id].brightness;
}

// Scale each channel by brightness/255 (rounded) and push all 8 pixels.
// Rewriting every pixel from the stored states preserves the other seven.
void applyLedsToHardware() {
  for (int i = 0; i < NUM_LEDS; i++) {
    uint16_t k = leds[i].brightness;
    uint8_t r = (leds[i].r * k + 127) / 255;
    uint8_t g = (leds[i].g * k + 127) / 255;
    uint8_t b = (leds[i].b * k + 127) / 255;
    bar.setPixelColor(i, bar.Color(r, g, b));
  }
  bar.show();
}

// ---------- Handlers ----------
void handleRoot() {
  server.send_P(200, "text/html", LAB_PAGE);
}

void handleDistance() {
  unsigned long echoUs = sonar.ping();          // blocking, bounded by MAX_DISTANCE_CM
  JsonDocument doc;
  if (echoUs == 0) {                            
    doc["distance_cm"] = nullptr;
    doc["valid"] = false;
  } else {
    double cm = echoUs / 2.0 * 0.0343;       
    doc["distance_cm"] = round(cm * 10.0) / 10.0; // 1 decimal place
    doc["valid"] = true;
  }
  String out;
  serializeJson(doc, out);
  sendJson(200, out);
}

void handleGetLeds() {
  JsonDocument doc;
  JsonArray arr = doc["leds"].to<JsonArray>();
  for (int i = 0; i < NUM_LEDS; i++) ledToJson(arr.add<JsonObject>(), i);
  String out;
  serializeJson(doc, out);
  sendJson(200, out);
}

void handlePutLed() {
  // 1) LED id from the URL: must be exactly one digit 0-7, else 404
  String idStr = server.pathArg(0);
  if (idStr.length() != 1 || idStr[0] < '0' || idStr[0] > '7') {
    sendError(404, "Unknown LED id '" + idStr + "' (valid ids are 0-7)");
    return;
  }
  int id = idStr[0] - '0';

  // 2) Body must be a JSON object, else 400
  if (!server.hasArg("plain") || server.arg("plain").length() == 0) {
    sendError(400, "Missing JSON body");
    return;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err || !doc.is<JsonObject>()) {
    sendError(400, "Malformed JSON body");
    return;
  }

  // 3) Validate ALL fields before changing anything (preserve state on failure)
  const char* keys[4] = {"r", "g", "b", "brightness"};
  int vals[4];
  for (int k = 0; k < 4; k++) {
    JsonVariant v = doc[keys[k]];
    if (!v.is<int>()) {
      sendError(400, String("Field '") + keys[k] + "' is missing or not an integer");
      return;
    }
    vals[k] = v.as<int>();
    if (vals[k] < 0 || vals[k] > 255) {
      sendError(400, String("Field '") + keys[k] + "' must be 0-255");
      return;
    }
  }

  // 4) Commit, drive the hardware, THEN reply with the resulting state
  leds[id] = {(uint8_t)vals[0], (uint8_t)vals[1], (uint8_t)vals[2], (uint8_t)vals[3]};
  applyLedsToHardware();

  JsonDocument resp;
  ledToJson(resp.to<JsonObject>(), id);
  String out;
  serializeJson(resp, out);
  sendJson(200, out);
}

void handleNotFound() {
  sendError(404, "No route for " + server.uri());
}

// ---------- Wi-Fi ----------
void startNetwork() {
  if (!USE_AP) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(STA_SSID, STA_PASS);
    Serial.printf("Joining \"%s\"", STA_SSID);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < STA_TIMEOUT_MS) {
      delay(250);
      Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("STA connected, RSSI %d dBm\n", WiFi.RSSI());
      Serial.printf("Dashboard URL: http://%s/\n", WiFi.localIP().toString().c_str());
      return;
    }
    Serial.println("STA connection failed - falling back to AP mode.");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 2);
  Serial.printf("AP \"%s\" up. Dashboard URL: http://%s/\n",
                AP_SSID, WiFi.softAPIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < NUM_LEDS; i++) leds[i] = {0, 0, 0, 128}; 
  bar.begin();
  applyLedsToHardware();

  startNetwork();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/distance", HTTP_GET, handleDistance);
  server.on("/api/leds", HTTP_GET, handleGetLeds);
  server.on(UriBraces("/api/leds/{}"), HTTP_PUT, handlePutLed);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  server.handleClient();   // never block here with delay()
}

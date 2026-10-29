/*
 * EE4216 Lab 3 - FINAL Gateway B (Tasks 1-4)
 * ESP32-S3, Arduino IDE, Serial Monitor 115200
 * 1) AP + STA network
 * 2) POST /api/sensor receives JSON and prints Singapore timestamp
 * 3) HTTPS POST /update uploads distance every ~20 sec
 * 4) HTTPS GET polls LED Control every ~5 sec, forwards to Node A
 *
 * Install ArduinoJson; built-in ESP32 WiFi/WebServer/HTTPClient libraries.
 * TLS checks the server certificate against the trusted CA list below.
 */
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

// ===== SETTINGS: CHANGE THESE BEFORE UPLOAD =====
const char* AP_SSID = "A0309529W";
const char* AP_PASSWORD = "YOUR_GATEWAY_AP_PASSWORD";   // Must match Sensor Node A
const char* STA_SSID = "YOUR_WIFI_SSID";
const char* STA_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your router password

// Fill the correct channel's key/ID from ThingSpeak -> API Keys.
// These are TWO DIFFERENT channels (Sensor Data and LED Control).
const char* SENSOR_DATA_WRITE_KEY = "YOUR_SENSOR_DATA_WRITE_API_KEY";
const char* LED_CONTROL_CHANNEL_ID = "YOUR_LED_CONTROL_CHANNEL_ID";
const char* LED_CONTROL_READ_KEY = "YOUR_LED_CONTROL_READ_API_KEY";
// The LED Control WRITE key is used from the ThingSpeak website/browser,
// not by the Gateway firmware.
// ================================================

const unsigned long SENSOR_UPLOAD_INTERVAL_MS = 20000UL;
const unsigned long CLOUD_POLL_INTERVAL_MS = 5000UL;
const unsigned long LED_RETRY_INTERVAL_MS = 3000UL;
const unsigned long WIFI_RETRY_INTERVAL_MS = 10000UL;
const char* TS_UPDATE_URL = "https://api.thingspeak.com/update";

// Trusted PUBLIC root certificates (no secrets, do not replace with setInsecure).
// HTTPS server validation also requires an NTP-synchronized system clock.
static const char TS_ROOT_CAS[] = R"ROOTCERT(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)ROOTCERT";

WebServer server(80);
float latestDistance = NAN;
unsigned long lastReceivedMillis = 0;
unsigned long readingSequence = 0;
unsigned long uploadedSequence = 0;
IPAddress sensorNodeIP(0, 0, 0, 0);
bool haveSensorIP = false;

unsigned long lastUploadAttempt = 0;
unsigned long lastCloudPoll = 0;
unsigned long lastLedAttempt = 0;
unsigned long lastWiFiRetry = 0;
bool upstreamPreviouslyConnected = false;

long lastCompletedEntryId = 0;
bool pendingLed = false;
long pendingEntryId = 0;
int pendingIndex = 0;
int pendingColorCode = 0;

#if ARDUINOJSON_VERSION_MAJOR >= 7
  #define NEW_JSON(name) JsonDocument name
#else
  #define NEW_JSON(name) StaticJsonDocument<512> name
#endif

bool cloudKeysConfigured() {
  return String(SENSOR_DATA_WRITE_KEY).length() > 0 &&
         String(SENSOR_DATA_WRITE_KEY) != "YOUR_SENSOR_DATA_WRITE_API_KEY";
}
bool ledChannelConfigured() {
  return String(LED_CONTROL_CHANNEL_ID).length() > 0 &&
         String(LED_CONTROL_CHANNEL_ID) != "YOUR_LED_CONTROL_CHANNEL_ID" &&
         String(LED_CONTROL_READ_KEY).length() > 0 &&
         String(LED_CONTROL_READ_KEY) != "YOUR_LED_CONTROL_READ_API_KEY";
}
bool clockIsReady() { return time(nullptr) > 1700000000; }

void printReceptionTime() {
  if (clockIsReady()) {
    time_t nowUtc = time(nullptr);
    struct tm localTime;
    localtime_r(&nowUtc, &localTime);
    char stamp[26];
    strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &localTime);
    Serial.printf("[%s SGT]", stamp);
  } else {
    Serial.printf("[uptime %lu ms]", millis());
  }
}

// TASK 2: Receive and validate Sensor Node's reading; remember its DHCP IP.
void handleSensor() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
    return;
  }
  NEW_JSON(doc);
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
    return;
  }
  JsonVariantConst v = doc["distance"];
  if (!(v.is<int>() || v.is<long>() || v.is<float>() || v.is<double>())) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid distance\"}");
    return;
  }
  float d = v.as<float>();
  if (!isfinite(d) || d <= 0.0f || d > 400.0f) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"distance out of range\"}");
    return;
  }
  latestDistance = d;
  lastReceivedMillis = millis();
  readingSequence++;
  sensorNodeIP = server.client().remoteIP();  // Actual Sensor A DHCP IP
  haveSensorIP = true;

  Serial.print("[TASK 2]");
  printReceptionTime();
  Serial.printf(" Sensor %s: %.2f cm\n", sensorNodeIP.toString().c_str(), d);
  server.send(200, "application/json", "{\"ok\":true}");
}

void maintainUpstreamWiFi() {
  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected && !upstreamPreviouslyConnected) {
    upstreamPreviouslyConnected = true;
    Serial.print("[CHECK-OFF 1] Gateway STA IP: ");
    Serial.println(WiFi.localIP());
    // GMT offset +8 h. NTP required for validating HTTPS certificates.
    configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com");
    Serial.println("[NTP] Time synchronization requested (Singapore UTC+8)");
  } else if (!connected && upstreamPreviouslyConnected) {
    upstreamPreviouslyConnected = false;
    Serial.println("[Wi-Fi] Upstream disconnected; AP remains available");
  }
  if (!connected && millis() - lastWiFiRetry >= WIFI_RETRY_INTERVAL_MS) {
    lastWiFiRetry = millis();
    WiFi.reconnect();
    Serial.println("[Wi-Fi] Retrying upstream Wi-Fi...");
  }
}

// TASK 3: Send distance to Sensor Data channel Field 1.
// A response body with a positive entry ID confirms storage.
bool uploadDistance(float distance) {
  WiFiClientSecure tls;
  tls.setCACert(TS_ROOT_CAS);
  tls.setHandshakeTimeout(8);
  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  if (!http.begin(tls, TS_UPDATE_URL)) {
    Serial.println("[TASK 3] HTTPS begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String body = "api_key=" + String(SENSOR_DATA_WRITE_KEY) +
                "&field1=" + String(distance, 2);
  int code = http.POST(body);
  bool success = false;
  if (code > 0) {
    String response = http.getString();
    response.trim();
    long entry = response.toInt();
    Serial.printf("[TASK 3] HTTP=%d, response=%s\n", code, response.c_str());
    if (code == HTTP_CODE_OK && entry > 0) {
      Serial.printf("[CHECK-OFF 3] Distance %.2f cm saved, entry_id=%ld\n",
                    distance, entry);
      success = true;
    } else {
      Serial.println("[TASK 3] Not saved (HTTP 200 with body '0' is failure)");
    }
  } else {
    Serial.printf("[TASK 3] HTTPS error: %s\n", http.errorToString(code).c_str());
  }
  http.end();
  return success;
}

// ThingSpeak JSON Field 1/2 may be strings, e.g. "3" and "1".
bool parseIntegerField(JsonVariantConst v, long &result) {
  if (v.is<int>() || v.is<long>()) {
    result = v.as<long>();
    return true;
  }
  if (v.is<const char*>()) {
    const char* s = v.as<const char*>();
    if (s == nullptr || *s == '\0') return false;
    char* end = nullptr;
    long n = strtol(s, &end, 10);
    if (end == s || *end != '\0') return false;
    result = n;
    return true;
  }
  return false;
}

// TASK 4: Read latest LED command. Do not mark complete until Node A ACKs.
void pollLedControl() {
  if (pendingLed) return; // Keep retrying the current unacknowledged command
  String url = "https://api.thingspeak.com/channels/" +
               String(LED_CONTROL_CHANNEL_ID) +
               "/feeds/last.json?api_key=" + String(LED_CONTROL_READ_KEY);
  WiFiClientSecure tls;
  tls.setCACert(TS_ROOT_CAS);
  tls.setHandshakeTimeout(8);
  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  if (!http.begin(tls, url)) {
    Serial.println("[TASK 4] HTTPS GET initialization failed");
    return;
  }
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    if (code > 0) Serial.printf("[TASK 4] Poll HTTP status=%d\n", code);
    else Serial.printf("[TASK 4] Poll error: %s\n", http.errorToString(code).c_str());
    http.end();
    return;
  }
  String response = http.getString();
  http.end();
  NEW_JSON(doc);
  if (deserializeJson(doc, response)) {
    Serial.println("[TASK 4] Invalid JSON from ThingSpeak");
    return;
  }
  long id, index, colorCode;
  if (!parseIntegerField(doc["entry_id"], id) || id <= 0 ||
      !parseIntegerField(doc["field1"], index) || index < 0 || index > 7 ||
      !parseIntegerField(doc["field2"], colorCode) || colorCode < 0 || colorCode > 3) {
    Serial.println("[TASK 4] Missing/invalid entry_id, LED index, or color code");
    return;
  }
  if (id <= lastCompletedEntryId) return;  // Duplicate cloud entry
  pendingLed = true;
  pendingEntryId = id;
  pendingIndex = (int)index;
  pendingColorCode = (int)colorCode;
  lastLedAttempt = millis() - LED_RETRY_INTERVAL_MS; // Forward immediately
  Serial.printf("[TASK 4] New command entry=%ld, LED=%d, color=%d\n",
                id, pendingIndex, pendingColorCode);
}

void colorCodeToRgb(int code, int &r, int &g, int &b) {
  r = g = b = 0;
  if (code == 1) r = 255;
  else if (code == 2) g = 255;
  else if (code == 3) b = 255;
}

bool forwardLedCommand() {
  if (!haveSensorIP) {
    Serial.println("[TASK 4] Waiting for Sensor A to report its IP via /api/sensor");
    return false;
  }
  int r, g, b;
  colorCodeToRgb(pendingColorCode, r, g, b);
  NEW_JSON(doc);
  doc["entry_id"] = pendingEntryId; // Optional id for idempotent retries
  doc["index"] = pendingIndex;
  doc["r"] = r;
  doc["g"] = g;
  doc["b"] = b;
  String body;
  serializeJson(doc, body);

  String url = "http://" + sensorNodeIP.toString() + "/api/led";
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(2500);
  http.setTimeout(2500);
  if (!http.begin(client, url)) {
    Serial.println("[TASK 4] Local HTTP begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  if (code != HTTP_CODE_OK) {
    if (code > 0) Serial.printf("[TASK 4] Node A returned HTTP %d\n", code);
    else Serial.printf("[TASK 4] Node A error: %s\n", http.errorToString(code).c_str());
    http.end();
    return false;
  }
  String reply = http.getString();
  http.end();
  NEW_JSON(ack);
  if (deserializeJson(ack, reply) ||
      !ack["ok"].as<bool>() || ack["entry_id"].as<long>() != pendingEntryId) {
    Serial.printf("[TASK 4] Missing/mismatched ACK: %s\n", reply.c_str());
    return false;
  }
  Serial.printf("[CHECK-OFF 4] Node A ACK: LED %d color %d, entry=%ld\n",
                pendingIndex, pendingColorCode, pendingEntryId);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.print("[CHECK-OFF 1] Gateway AP IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[ERROR] Cannot start Gateway AP");
  }
  // Local sensor server runs even when the upstream Internet is down.
  server.on("/api/sensor", HTTP_POST, handleSensor);
  server.begin();
  Serial.println("[TASK 2] HTTP server ready: POST /api/sensor");

  WiFi.begin(STA_SSID, STA_PASSWORD);
  lastWiFiRetry = millis();
  Serial.println("[TASK 1] Connecting upstream Wi-Fi...");
  if (!cloudKeysConfigured()) Serial.println("[CONFIG] Fill SENSOR_DATA_WRITE_KEY");
  if (!ledChannelConfigured()) Serial.println("[CONFIG] Fill LED_CONTROL_CHANNEL_ID and LED_CONTROL_READ_KEY");
}

void loop() {
  server.handleClient();
  maintainUpstreamWiFi();
  unsigned long now = millis();

  if (now - lastUploadAttempt >= SENSOR_UPLOAD_INTERVAL_MS) {
    lastUploadAttempt = now;
    if (cloudKeysConfigured() && WiFi.status() == WL_CONNECTED &&
        clockIsReady() && readingSequence > uploadedSequence) {
      unsigned long sequence = readingSequence;
      if (uploadDistance(latestDistance)) uploadedSequence = sequence;
    }
  }

  if (now - lastCloudPoll >= CLOUD_POLL_INTERVAL_MS) {
    lastCloudPoll = now;
    if (ledChannelConfigured() && WiFi.status() == WL_CONNECTED && clockIsReady()) {
      pollLedControl();
    }
  }

  if (pendingLed && now - lastLedAttempt >= LED_RETRY_INTERVAL_MS) {
    lastLedAttempt = now;
    if (forwardLedCommand()) {
      lastCompletedEntryId = pendingEntryId;
      pendingLed = false; // Complete only after successful acknowledgment
    }
  }
  delay(2);
}

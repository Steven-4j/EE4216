#include <WiFi.h>

const char* AP_SSID = "A0305871A";  
const char* AP_PASS = "12345678";    
const int   AP_CHANNEL = 1;
const int   AP_HIDDEN = 0;            
const int   AP_MAX_CONN = 2;      

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONN);

  if (!ok) {
    Serial.println("softAP() FAILED");
    return;
  }
  Serial.println("=== Soft AP started ===");
  Serial.printf("AP SSID         : %s\n", AP_SSID);
  Serial.printf("AP IP address   : %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("AP MAC address  : %s\n", WiFi.softAPmacAddress().c_str());
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last >= 5000) {
    last = millis();
    Serial.printf("Connected stations: %d / %d\n", WiFi.softAPgetStationNum(), AP_MAX_CONN);
  }
}

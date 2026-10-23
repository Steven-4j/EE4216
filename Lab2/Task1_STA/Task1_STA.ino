#include <WiFi.h>

const char* SSID = "TP-Link_6FCB";  
const char* PASS = "72972786";
const uint32_t TIMEOUT = 15000;   

bool connectSTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  Serial.printf("Connecting to \"%s\"", SSID);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < TIMEOUT) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  Serial.begin(115200);
  delay(1000); 

  if (connectSTA()) {
    Serial.println("=== Connection result: SUCCESS ===");
    Serial.printf("Network SSID     : %s\n", WiFi.SSID().c_str());
    Serial.printf("RSSI             : %d dBm\n", WiFi.RSSI());
    Serial.printf("STA IP address   : %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("STA MAC address  : %s\n", WiFi.macAddress().c_str());
  } else {
    Serial.println("=== Connection result: FAILED ===");
    Serial.printf("Gave up after %lu ms. Last status: %s\n",
                  (unsigned long)TIMEOUT, WiFi.status());
  }
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last >= 5000) {
    last = millis();
    Serial.printf("status=%d  RSSI=%d dBm\n",
                  WiFi.status(), WiFi.RSSI());
  }
}

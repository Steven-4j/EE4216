# EE4216 Lab 3: Bidirectional IoT Gateway Using HTTP

Group 5: Steven Ang Jaya (A0305871A) and Huang Shangyi (A0309529W).
Group submission repository: <https://github.com/Eason-Huangsy/EE4216-Lab3>

Two ESP32-S3 boards link a local sensor network to ThingSpeak:

- **Board A, sensor node** (my part): joins the gateway AP, sends HC-SR04 distance readings to the gateway every 5 s, and serves `POST /api/led` to set one NeoPixel.
- **Board B, gateway** (written by Huang Shangyi, copied here for reference): runs in AP+STA mode, uploads readings to ThingSpeak over HTTPS, polls the LED Control channel and forwards new commands to Board A.

| Folder | Contents |
| --- | --- |
| `Sensor_Node_A_Task1` | Join the gateway AP and print the DHCP IP (Task 1) |
| `Sensor_Node_A_Task2` | Task 1 + POST distance JSON to `/api/sensor` every 5 s (Task 2) |
| `Sensor_Node_A_Final` | Tasks 1, 2 and 4: telemetry + `POST /api/led` with duplicate `entry_id` handling |
| `Gateway_B_Final` | Gateway B, Tasks 1–4 (by Huang Shangyi) |

## Wiring on Board A

| Connection | GPIO |
| --- | --- |
| HC-SR04 TRIG | 4 |
| HC-SR04 ECHO | 5, through the Lab 1 voltage divider |
| NeoPixel DIN | 7 |

## Configuration

Replace the `YOUR_...` placeholders before uploading: the gateway AP password (same in both boards), the router SSID/password, and the ThingSpeak keys in the gateway sketch. Keep real passwords and API keys out of GitHub.

Libraries: ArduinoJson, Adafruit NeoPixel. Board: ESP32S3 Dev Module. Serial Monitor: 115200 baud.

#include <esp_now.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

// ===== WiFi Config =====
const char* ssid = "laptop";
const char* password = "laptop11";

// ===== BLE Config =====
#define SCAN_TIME 1 // Scan duration
#define RUN_DURATION 60000 // 2 minutes in ms
BLEScan* pBLEScan;
String targetMac = "";

// ===== Anchor ID (set different for each ESP32) =====
#define ANCHOR_ID 1  // Change: 1, 2, 3

// ===== Collector MAC (Server ESP32) =====
uint8_t serverMAC[] = {0x38, 0x18, 0x2B, 0x83, 0xDA, 0x54};

// Structs
typedef struct {
  char mac[18]; // target MAC from server
} TargetMac;

typedef struct {
  int senderID;
  char mac[18];
  float avgRssi;
  float avgDistance;
  float kalmanRssi;
  float kalmanDistance;
} ScanResult;

TargetMac incomingTarget;
ScanResult myData;

unsigned long startTime;
bool scanningActive = false;
int rssiSum = 0, rssiCount = 0;

// Buffer to store RSSI samples for Kalman
#define MAX_SAMPLES 512
int rssiValues[MAX_SAMPLES];
int rssiIndex = 0;

// Distance function
float calculateDistance(float rssi, int a = -57.2) {
  return pow(10, ((a - rssi) / (10 *2.068400972)));
}

// ===== ESP-NOW Receive Callback =====
void onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  if (len != sizeof(TargetMac)) return;
  memcpy(&incomingTarget, incomingData, sizeof(incomingTarget));

  targetMac = String(incomingTarget.mac);
  targetMac.toLowerCase();

  Serial.println("Received target: " + targetMac);
  startTime = millis();
  rssiSum = 0; rssiCount = 0;
  rssiIndex = 0; // reset buffer
  scanningActive = true;
}

// ===== ESP-NOW Send Callback ====
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Result sent OK" : "Result send failed");
}

void setup() {
  Serial.begin(115200);
   WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

 

  // BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(50);

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
  esp_now_register_send_cb(onSent);

  // Add server as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, serverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("Anchor ready. Waiting for target MAC...");
}

void loop() {
  if (scanningActive) {
    if (millis() - startTime < RUN_DURATION) {
      BLEScanResults* results = pBLEScan->start(SCAN_TIME, false);
      for (int i = 0; i < results->getCount(); i++) {
        BLEAdvertisedDevice d = results->getDevice(i);
        String mac = d.getAddress().toString().c_str();
        mac.toLowerCase();
        if (mac == targetMac) {
          int rssi = d.getRSSI();
          rssiSum += rssi;
          rssiCount++;
          // store sample for Kalman (bounded)
          if (rssiIndex < MAX_SAMPLES) {
            rssiValues[rssiIndex++] = rssi;
          }
          Serial.printf("Anchor %d detected %s | RSSI %d\n", ANCHOR_ID, mac.c_str(), rssi);
        }
      }
      pBLEScan->clearResults();
      delay(500);
    } else {
      scanningActive = false;
      if (rssiCount > 0) {
        float avgRssi = (float)rssiSum / rssiCount;
        float avgDist = calculateDistance(avgRssi);

        // --- Kalman filter (from your attached images) ---
        float Q = 1e-6;   // Process noise
        float R = 0.01;   // Measurement noise
        float A = 1;      // State transition
        float C = 1;      // Measurement
        float P = 1;      // Error covariance
        float x_hat = 0;  // Current estimate

        float kalmanRssi = avgRssi; // fallback if single sample
        if (rssiIndex > 0) {
          // initialize with first sample
          x_hat = (float)rssiValues[0];
          P = 1;
          for (int k = 1; k < rssiIndex; k++) {
            // Prediction
            float x_pred = A * x_hat;
            float P_pred = A * P + Q;

            // Kalman Gain
            float K = P_pred / (P_pred + R);

            // Update with measurement
            x_hat = x_pred + K * ((float)rssiValues[k] - C * x_pred);

            // Update covariance
            P = (1 - K * C) * P_pred;
          }
          kalmanRssi = x_hat;
        }

        float kalmanDist = calculateDistance(kalmanRssi);

        myData.senderID = ANCHOR_ID;
        strncpy(myData.mac, targetMac.c_str(), sizeof(myData.mac));
        myData.mac[sizeof(myData.mac)-1] = '\0';
        myData.avgRssi = avgRssi;
        myData.avgDistance = avgDist;
        myData.kalmanRssi = kalmanRssi;
        myData.kalmanDistance = kalmanDist;

        Serial.printf("Anchor %d final avg RSSI %.2f, dist %.2f m\n", ANCHOR_ID, avgRssi, avgDist);
        Serial.printf("Anchor %d kalman RSSI %.2f, dist %.2f m\n", ANCHOR_ID, kalmanRssi, kalmanDist);
        esp_now_send(serverMAC, (uint8_t*)&myData, sizeof(myData));
      } else {
        Serial.println("No target detected in 2 mins");
      }
    }
  }
}

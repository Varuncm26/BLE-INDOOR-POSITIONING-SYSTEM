#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <math.h>  // Needed for pow()

// BLE Configuration
#define SCAN_TIME 2        // Duration to scan for BLE devices (seconds)
#define RUN_DURATION 60000 // Run for 20 seconds (milliseconds)

// Target MAC address (lowercase)
String targetMac = "3c:8a:1f:0c:69:06"; // Replace with your MAC 0x3C, 0x8A, 0x1F, 0x0C, 0x69, 0x06
String targetMac2 = "05:24:72:01:bc:20";
String targetMac3 = "38:18:2b:83:da:56";


// Function to estimate distance based on RSSI
float calculateDistance(float rssi, int a = -41.1603) {
  return pow(10, ((a - rssi) / (10 * 4.1237)));
}

// Timer variable
unsigned long startTime;

// Variables to store RSSI data
int rssiSum = 0;
int rssiCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("BLE Scanner Starting...");
  BLEDevice::init("");
  startTime = millis(); // Record start time
}

void loop() {
  // If 20 seconds are completed
  if (millis() - startTime >= RUN_DURATION) {
    if (rssiCount > 0) {
      float avgRssi = (float)rssiSum / rssiCount;
      float avgDistance = calculateDistance(avgRssi);
      Serial.println("=== Final Results After 20 Seconds ===");
      Serial.printf(" %.2f \n", avgRssi);
      Serial.printf("Estimated Distance: %.2f meters\n", avgDistance);
    } else {
      Serial.println("No target device detected in 20 seconds.");
    }
    while (true) { delay(100); } // Stop program
  }

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(50);

  BLEScanResults* scanResults = pBLEScan->start(SCAN_TIME, false);
  int deviceCount = scanResults->getCount();

  for (int i = 0; i < deviceCount; i++) {
    BLEAdvertisedDevice device = scanResults->getDevice(i);

    String mac = device.getAddress().toString().c_str();
    mac.toLowerCase(); // Case-insensitive comparison
//  || mac == targetMac2)
    if (mac == targetMac || mac == targetMac2 || mac == targetMac3) {
      int rssi = device.getRSSI();
      rssiSum += rssi;
      rssiCount++;

      Serial.printf(" %d \n", rssi);
    }
  }

  pBLEScan->clearResults();
  delay(1000);
}

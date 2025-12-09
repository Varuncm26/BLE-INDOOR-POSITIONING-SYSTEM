#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <math.h>  // Needed for pow()

// BLE Configuration
#define SCAN_TIME 5  // Duration to scan for BLE devices (in seconds)

// Function to estimate distance based on RSSI
float calculateDistance(int rssi, int a = -45.0021) {
 return pow(10, ((a - rssi) / (10*2.3778)));
}

 void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("BLE Scanner Starting...");

  // Initialize BLE
  BLEDevice::init("");
}

void loop() {
  // Get BLE scanner instance
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);  // Get more info like device name
  pBLEScan->setInterval(100);     // Scan interval in milliseconds
  pBLEScan->setWindow(100);        // Scan window duration in milliseconds

  // Start scanning
  Serial.printf("Scanning for BLE devices for %d seconds...\n", SCAN_TIME);
  BLEScanResults* scanResults = pBLEScan->start(SCAN_TIME, false);

  int deviceCount = scanResults->getCount();
  Serial.printf("Scan complete. Found %d device(s):\n", deviceCount);

  // Loop through found devices
  for (int i = 0; i < deviceCount; i++) {
    BLEAdvertisedDevice device = scanResults->getDevice(i);

    // Get device name (fallback to "Unknown")
    String deviceName = device.haveName() ? device.getName().c_str() : "Unknown";
    String mac = device.getAddress().toString().c_str();
    int rssi = device.getRSSI();
    float distance = calculateDistance(rssi);

    // Print to Serial
    Serial.printf("Device %d:\n", i + 1);
    Serial.printf("  Name     : %s\n", deviceName.c_str());
    Serial.printf("  MAC      : %s\n", mac.c_str());
    Serial.printf("  RSSI     : %d dBm\n", rssi);
    Serial.printf("  Distance : %.2f meters\n\n", distance);
  }

  // Free memory used by scan results
  pBLEScan->clearResults();

  // Delay before next scan
  delay(100);  // 5 seconds pause before next scan
}
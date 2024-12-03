#include "FirebaseHandler.h"
#include "RelayManager.h"
#include "WifiHandler.h"
#include <Arduino.h>

void setup() {
  // 115200 or 9600 or 460800
  Serial.begin(460800);

  // Initialize WiFi
  WiFiHandler::connectToWiFi();

  // Define relay
  RelayManager::initializeRelays();

  // Initialize Firebase
  FirebaseHandler::begin();

  delay(2000);
}

void loop() {
  // Update FirebaseHandler periodically
  FirebaseHandler::update();
}

#include "FirebaseHandler.h"
#include "RelayManager.h"
#include "SensorManager.h"
#include "WifiHandler.h"
#include <Arduino.h>

void setup() {
  // 115200, 9600, 460800
  Serial.begin(460800);

  // Initialize WiFi
  WiFiHandler::connectToWiFi();

  // Intialize relays and sensors
  RelayManager::activateRelays();
  SensorManager::initializeSensors();

  // Initialize Firebase
  FirebaseHandler::begin();

  delay(2000);
}

void loop() {
  FirebaseHandler::loop();

  FirebaseHandler::processRelayStreamData();
  FirebaseHandler::manageMetricsData();

  SensorManager::updateFlameStatus();
  SensorManager::updateSmokeStatus();
  RelayManager::buzzerBlink();
}

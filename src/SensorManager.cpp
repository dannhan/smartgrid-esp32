#include "SensorManager.h"
#include "FirebaseHandler.h"
#include "RelayManager.h"

int SensorManager::flameStatus = LOW;
int SensorManager::smokeStatus = HIGH;
unsigned long SensorManager::flameMs = 0;
unsigned long SensorManager::smokeMs = 0;

void SensorManager::initializeSensors() {
  pinMode(sensorApi, INPUT);
  pinMode(sensorAsap, INPUT);
}

void SensorManager::updateFlameStatus() {
  const int newFlameStatus = digitalRead(sensorApi);

  // Do nothing if the flame status remains the same and millis not past 5000
  if (flameStatus == newFlameStatus)
    return;

  Serial.println("Flame status changed");
  if (millis() - smokeMs > 1000) {
    flameStatus = newFlameStatus;
    Serial.println("Flame status: " + String(newFlameStatus));
    if (flameStatus == HIGH) {
      RelayManager::flameDetectedCallback();
      // change notification status
      FirebaseHandler::setSensorStatus("flame", HIGH);
    } else {
      RelayManager::flameClearedCallback();
      // change notification status
      FirebaseHandler::setSensorStatus("flame", LOW);
    }
  }
}

void SensorManager::updateSmokeStatus() {
  const int newSmokeStatus = digitalRead(sensorAsap);

  // Do nothing if the smoke status remains the same
  if (smokeStatus == newSmokeStatus)
    return;

  Serial.println("Smoke status changed");
  if (millis() - smokeMs > 1000) {
    smokeMs = millis();
    smokeStatus = newSmokeStatus;
    Serial.println("Smoke status: " + String(newSmokeStatus));
    if (smokeStatus == LOW) {
      RelayManager::smokeDetectedCallback();
      // change notification status
      FirebaseHandler::setSensorStatus("smoke", LOW);
    } else {
      RelayManager::smokeClearedCallback();
      // change notification status
      FirebaseHandler::setSensorStatus("smoke", HIGH);
    }
  }
}

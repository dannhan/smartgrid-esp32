#pragma once
#include <Arduino.h>
#include <FirebaseClient.h>

class SensorManager {
public:
  static void initializeSensors();
  static void updateFlameStatus();
  static void updateSmokeStatus();

  static int flameStatus;
  static int smokeStatus;

private:
  static unsigned long flameMs;
  static unsigned long smokeMs;

  static constexpr const int sensorApi = 5;
  static constexpr const int sensorAsap = 18;
};

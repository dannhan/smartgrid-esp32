#pragma once
#include <Arduino.h>
#include <FirebaseClient.h>

class RelayManager {
public:
  static void activateRelays();
  static void initializeRelayState(RealtimeDatabaseResult RTDB);
  static void handleRelayUpdate(RealtimeDatabaseResult RTDB);
  static void flameDetectedCallback();
  static void flameClearedCallback();
  static void smokeDetectedCallback();
  static void smokeClearedCallback();

  static void buzzerBlink();
  static bool isBuzzerBlinkActive;

private:
  // clang-format off
  // static constexpr const int relaiSatu    = 2;
  // static constexpr const int relaiDua     = 4;
  // static constexpr const int relaiTiga    = 5;
  // static constexpr const int relaiEmpat   = 18;
  // static constexpr const int relaiLima    = 19;
  // static constexpr const int relaiEnam    = 21; 
  // static constexpr const int relaiTujuh   = 22;
  // static constexpr const int relaiDelapan = 23;
  
  static constexpr const int buzzer = 19;
  static constexpr const int relaiPompa   = 15;
  
  static constexpr const int relaiSatu    = 13;
  static constexpr const int relaiDua     = 12;
  static constexpr const int relaiTiga    = 14;
  static constexpr const int relaiEmpat   = 27;
  static constexpr const int relaiLima    = 26;
  static constexpr const int relaiEnam    = 25;
  static constexpr const int relaiTujuh   = 33;
  static constexpr const int relaiDelapan = 32;
  // clang-format on
  static unsigned long buzzerBlinkMs;

  static void turnOffLampsAndSockets();
};

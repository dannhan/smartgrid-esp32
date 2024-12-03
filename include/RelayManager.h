#pragma once
#include <Arduino.h>
#include <FirebaseClient.h>

class RelayManager {
public:
  static void initializeRelays();
  static void initializeState(RealtimeDatabaseResult RTDB);
  static void handleRelayUpdate(AsyncResult &aResult);

private:
  // clang-format off
  static constexpr const int relaiSatu    = 2;
  static constexpr const int relaiDua     = 4;
  static constexpr const int relaiTiga    = 5;
  static constexpr const int relaiEmpat   = 18;
  static constexpr const int relaiLima    = 19;
  static constexpr const int relaiEnam    = 21;
  static constexpr const int relaiTujuh   = 22;
  static constexpr const int relaiDelapan = 23;
  // TODO: static constexpr const int relaiPompa = 23;

  // clang-format on
};

#pragma once
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include <map>

class MonitorData {
public:
  static std::map<int, String> RealtimeVoltagesKey;
  static std::map<int, String> HourlyVoltagesKey;
  static std::map<int, String> RealtimeCurrentsKey;
  static std::map<int, String> HourlyCurrentsKey;
  static std::map<int, String> RealtimePowersKey;
  static std::map<int, String> HourlyPowersKey;
};

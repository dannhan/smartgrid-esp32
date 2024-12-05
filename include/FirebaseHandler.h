#pragma once
#include <ArduinoJson.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

class FirebaseHandler {
public:
  static void begin();
  static void loop();
  static void processRelayStreamData();
  static void manageMetricsData();
  static void setSensorStatus(String sensorType, int value);

private:
  // Firebase and networking objects
  static WiFiClientSecure ssl_client1, ssl_client2;
  static DefaultNetwork network;
  static AsyncClientClass aClient, aClient2;
  static FirebaseApp app;
  static RealtimeDatabase Database;
  static AsyncResult aResult_no_callback, aResult_no_callback2;
  static LegacyToken legacy_token;

  static void pushRealtimeMetricsData();
  static void deleteRealtimeMetricsData(AsyncResult &aResult);
  static void deleteFirstIndex(const char *path, JsonObject);

  static void updateTimer();
  static void printResult(AsyncResult &aResult);

  static unsigned long ms;
  static unsigned long second, minute, hour;
  static unsigned int realtimeDataLength;
  static bool isDeletingMetricData;
};

#pragma once
#include <ArduinoJson.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

// TODO: separation of concern
class FirebaseHandler {
public:
  static void begin();
  static void update();

private:
  // Firebase and networking objects
  static WiFiClientSecure ssl_client1, ssl_client2;
  static DefaultNetwork network;
  static AsyncClientClass aClient, aClient2;
  static FirebaseApp app;
  static RealtimeDatabase Database;
  static AsyncResult aResult_no_callback, aResult_no_callback2;
  static LegacyToken legacy_token;

  static void pushData(const char *path);
  static void updateTimer();
  static void printResult(AsyncResult &aResult);
  static void deleteOldestKey(JsonDocument &doc, const char *path);
  static void deleteOldestKeys(AsyncResult &aResult);

  static unsigned long ms;
  static unsigned long second, minute, hour;
  static unsigned int realtimeDataLength;
  static bool deleteCompleted;

  // Dump:
  // static object_t randGenerator();
  // static void getOldestKey(AsyncResult &aResult, const char *path);
  // static void syncCallback(AsyncResult &aResult);
  // static void asyncCallback(AsyncResult &aResult);
};

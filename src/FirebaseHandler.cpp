#include "FirebaseHandler.h"
#include "ArduinoJson/Json/JsonSerializer.hpp"
#include "HardwareSerial.h"
#include "RelayManager.h"
#include "config.h"
#include <PZEM004Tv30.h>

// Define static variables
unsigned long FirebaseHandler::ms = 0;
unsigned long FirebaseHandler::second = 0;
unsigned long FirebaseHandler::minute = 42;
unsigned long FirebaseHandler::hour = 13;
unsigned int FirebaseHandler::realtimeDataLength = 0;
bool FirebaseHandler::isDeletingMetricData = false;

// clang-format off
WiFiClientSecure FirebaseHandler::ssl_client1, FirebaseHandler::ssl_client2;
DefaultNetwork FirebaseHandler::network;
AsyncClientClass FirebaseHandler::aClient(FirebaseHandler::ssl_client1, getNetwork(FirebaseHandler::network));
AsyncClientClass FirebaseHandler::aClient2(FirebaseHandler::ssl_client2, getNetwork(FirebaseHandler::network));
FirebaseApp FirebaseHandler::app;
RealtimeDatabase FirebaseHandler::Database;
AsyncResult FirebaseHandler::aResult_no_callback, FirebaseHandler::aResult_no_callback2;
LegacyToken FirebaseHandler::legacy_token(DB_SECRET);
// clang-format on

/* -- Local Function Declaration -------------------- */
float zeroIfNan(float v);

// add perfix "0" if the number is less than 10
String padZero(int num);
object_t dataGenerator(String time, String type);
void printError(AsyncResult aResult);

/* -- Class Public Method -------------------------------- */

// Initializing the firebase client
void FirebaseHandler::begin() {
  Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  // i don't know what is the purpose of this?
  ssl_client1.setInsecure();
  ssl_client2.setInsecure();

  // Initialize the authentication handler.
  initializeApp(aClient2, app, getAuth(legacy_token));

  // Binding the authentication handler with your Database class object.
  app.getApp<RealtimeDatabase>(Database);

  // Initialize Firebase and stream settings
  Database.url(DB_URL);

  // filter the Stream events
  Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

  // Start streaming on control and keep track for the value change
  Database.get(aClient, "/control/", aResult_no_callback, true);

  // Get the length of realtime metric data and assigns it to realtimeDataLength
  String monitor = Database.get<String>(aClient2, "/monitor/voltages/realtime");
  JsonDocument doc;
  deserializeJson(doc, monitor);
  realtimeDataLength = doc.size();
  Serial.printf("The length of initial realtime data: %u\n",
                realtimeDataLength);
}

void FirebaseHandler::loop() {
  app.loop();
  Database.loop();
}

void FirebaseHandler::processRelayStreamData() {
  if (aResult_no_callback.available()) {
    RealtimeDatabaseResult &RTDB =
        aResult_no_callback.to<RealtimeDatabaseResult>();

    if (RTDB.isStream()) {
      const String path = RTDB.dataPath();

      if (path == "/") {
        RelayManager::initializeRelayState(RTDB);
      } else {
        Firebase.printf("task: %s\n", aResult_no_callback.uid().c_str());
        Serial.println("----------------------------");
        Firebase.printf("event: %s\n", RTDB.event().c_str());
        Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
        Firebase.printf("data: %s\n", RTDB.to<const char *>());
        Firebase.printf("type: %d\n", RTDB.type());

        RelayManager::handleRelayUpdate(RTDB);
      }
    } else {
      Serial.println("----------------------------");
      Firebase.printf("task: %s, payload: %s\n",
                      aResult_no_callback.uid().c_str(),
                      aResult_no_callback.c_str());
    }

    Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
  }
}

void FirebaseHandler::manageMetricsData() {
  // Push chart data if the data stored is less than 24
  if (millis() - ms > 5000 && app.ready() && realtimeDataLength <= 24 &&
      !isDeletingMetricData) {
    ms = millis();
    pushRealtimeMetricsData();

    Serial.println("realtimeDataLength: " + String(realtimeDataLength));
  }

  // Delete unused data if the data stored is more than 24 and the previous
  // deletion is completed
  if (realtimeDataLength > 24 && !isDeletingMetricData) {
    isDeletingMetricData = true;

    // get the value for monitor to be deleted
    DatabaseOptions options;
    options.filter.orderBy("$key");

    Database.get(aClient2, "/monitor/", options, aResult_no_callback2);
  }

  // This only run if aResult is available, which mean after
  // getting the values of old keys
  if (aResult_no_callback2.available()) {
    deleteRealtimeMetricsData(aResult_no_callback2);
  }
}

void FirebaseHandler::setSensorStatus(String sensorType, int value) {
  Database.set<int>(aClient2, "/sensors/" + sensorType, value);
}

/* -- Class Private Method -------------------------------- */
void FirebaseHandler::pushRealtimeMetricsData() {
  updateTimer();

  const String time = padZero(minute) + ":" + padZero(second);

  // clang-format off
  // TODO: this code is very blocking
  Database.push<object_t>(aClient2, "/monitor/voltages/realtime", dataGenerator(time, "voltage"));
  Database.push<object_t>(aClient2, "/monitor/currents/realtime", dataGenerator(time, "current"));
  Database.push<object_t>(aClient2, "/monitor/powers/realtime", dataGenerator(time, "energy"));
  // clang-format on

  realtimeDataLength++;
}

void FirebaseHandler::deleteRealtimeMetricsData(AsyncResult &aResult) {
  // Parse the async result
  RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
  JsonDocument doc;
  deserializeJson(doc, RTDB.to<const char *>());

  // delete the data
  deleteFirstIndex("/monitor/powers/realtime/", doc["powers"]["realtime"]);
  deleteFirstIndex("/monitor/voltages/realtime/", doc["voltages"]["realtime"]);
  deleteFirstIndex("/monitor/currents/realtime/", doc["currents"]["realtime"]);

  realtimeDataLength--;
  isDeletingMetricData = false;
}

void FirebaseHandler::deleteFirstIndex(const char *path, JsonObject json) {
  JsonObject::iterator it = json.begin();
  const char *firstKey = it->key().c_str();
  Serial.println("------ Now removing: " + String(path) + firstKey + "------");
  Database.remove(aClient2, String(path) + firstKey, printResult,
                  "databaseRemove");
};

// TODO: implement real timestamp functionality
void FirebaseHandler::updateTimer() {
  second += 20;
  if (second >= 60) {
    second = 0;
    minute++;
  }
  if (minute >= 60) {
    minute = 0;
    hour++;
  }
  if (hour >= 24) {
    hour = 0;
  }
}

void FirebaseHandler::printResult(AsyncResult &aResult) {
  if (aResult.isEvent()) {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(), aResult.appEvent().message().c_str(),
                    aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(),
                    aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(), aResult.error().message().c_str(),
                    aResult.error().code());
  }

  if (aResult.available()) {
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
    if (RTDB.isStream()) {
      Serial.println("----------------------------");
      Firebase.printf("task: %s\n", aResult.uid().c_str());
      Firebase.printf("event: %s\n", RTDB.event().c_str());
      Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
      Firebase.printf("data: %s\n", RTDB.to<const char *>());
      Firebase.printf("type: %d\n", RTDB.type());
    } else {
      Serial.println("----------------------------");
      Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(),
                      aResult.c_str());
    }

    Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
  }
}

/* -- Local Function Definition -------------------- */
void printError(AsyncResult aResult) {
  Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(),
                  aResult.error().message().c_str(), aResult.error().code());
}

// TODO: change this into data entry
PZEM004Tv30 pzem1(Serial2, 16, 17);

float zeroIfNan(float v) { return isnan(v) ? 0 : v; }

object_t dataGenerator(String time, String type) {
  char json[256];
  JsonDocument doc;

  doc["time"] = time;

  if (type == "voltage") {
    const float voltage = zeroIfNan(pzem1.voltage());
    Serial.printf("Voltage        : %.2f V\n", voltage);
    doc["value"] = voltage;
  } else if (type == "current") {
    const float current = zeroIfNan(pzem1.current());
    doc["value"] = zeroIfNan(pzem1.current());
    Serial.printf("Current        : %.2f A\n", current);
  } else if (type == "energy") {
    const float energy = zeroIfNan(pzem1.energy());
    doc["value"] = zeroIfNan(pzem1.energy());
    Serial.printf("Energy         : %.2f kWh\n", energy);
  }

  serializeJson(doc, json);
  return object_t(json);
}

String padZero(int num) { return num < 10 ? "0" + String(num) : String(num); }

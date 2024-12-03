#include "FirebaseHandler.h"
#include "HardwareSerial.h"
#include "RelayManager.h"
#include "config.h"

// Define static variables
unsigned long FirebaseHandler::ms = 0;
unsigned long FirebaseHandler::second = 0;
unsigned long FirebaseHandler::minute = 42;
unsigned long FirebaseHandler::hour = 13;
unsigned int FirebaseHandler::realtimeDataLength = 0;
bool FirebaseHandler::deleteCompleted = true;

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

// add perfix "0" if the number is less than 10
String padZero(int num);
object_t randGenerator(String time);
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

  // Get the length of data
  String monitor = Database.get<String>(aClient2, "/monitor/voltages/realtime");
  JsonDocument doc;
  deserializeJson(doc, monitor);
  realtimeDataLength = doc.size();
  Serial.printf("The length of initial realtime data: %u\n",
                realtimeDataLength);

  // filter the Stream events
  Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

  // Start streaming on control and keep track for the value change
  Database.get(aClient, "/control/", aResult_no_callback, true);
}

void FirebaseHandler::update() {
  app.loop();
  Database.loop();

  // Push chart data if the data stoerd is less than 24
  if (millis() - ms > 5000 && app.ready() && realtimeDataLength <= 24 &&
      deleteCompleted) {
    ms = millis();

    // clang-format off
    Database.push<object_t>(aClient2, "/monitor/voltages/realtime", randGenerator(padZero(minute) + ":" + padZero(second)));
    Database.push<object_t>(aClient2, "/monitor/currents/realtime", randGenerator(padZero(minute) + ":" + padZero(second)));
    Database.push<object_t>(aClient2, "/monitor/powers/realtime", randGenerator(padZero(minute) + ":" + padZero(second)));
    updateTimer();

    /* 
    Database.push<object_t>(aClient2, "/monitor/voltages/hourly", randGenerator());
    Database.push<object_t>(aClient2, "/monitor/currents/hourly", randGenerator());
    Database.push<object_t>(aClient2, "/monitor/powers/hourly", randGenerator());
    */
    // clang-format on

    realtimeDataLength++;
  }

  // Delete unused data if the data stored is more than 24 and the previous
  // deletion is completed
  if (realtimeDataLength > 24 && deleteCompleted) {
    deleteCompleted = false;

    DatabaseOptions options;
    options.filter.orderBy("$key");
    Database.get(aClient2, "/monitor/", options, aResult_no_callback2);
  }

  // Callback for streaming data
  RelayManager::handleRelayUpdate(aResult_no_callback);

  // TODO: damn what is this?
  deleteOldestKeys(aResult_no_callback2);
}

/* -- Class Private Method -------------------------------- */

void FirebaseHandler::pushData(const char *path) {
  Database.push<object_t>(
      aClient2, path, randGenerator(padZero(minute) + ":" + padZero(second)));
}

void FirebaseHandler::deleteOldestKeys(AsyncResult &aResult) {
  if (aResult.isError()) {
    printError(aResult);
  }

  if (aResult.available()) {
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();

    JsonDocument doc;
    deserializeJson(doc, RTDB.to<const char *>());
    // serializeJsonPretty(doc, Serial);

    // Call deleteOldestKey for each type, only specifying the base name
    deleteOldestKey(doc, "voltages");
    deleteOldestKey(doc, "currents");
    deleteOldestKey(doc, "powers");

    realtimeDataLength--;
    deleteCompleted = true;
  }
}

void FirebaseHandler::deleteOldestKey(JsonDocument &doc, const char *type) {
  String path = "/monitor/" + String(type) + "/realtime/";

  JsonObject json = doc[type]["realtime"].as<JsonObject>();

  while (json.size() > 24) {
    Serial.println(path + " JSON SIZE: " + String(json.size()));

    JsonObject::iterator it = json.begin();
    const char *firstKey = it->key().c_str();

    // Remove from JSON object and Firebase
    Database.remove(aClient2, path + firstKey, printResult, "databaseRemove");
    json.remove(firstKey);
  }
}

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

String padZero(int num) { return num < 10 ? "0" + String(num) : String(num); }

void printError(AsyncResult aResult) {
  Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(),
                  aResult.error().message().c_str(), aResult.error().code());
}

// TODO: change this into data entry
object_t randGenerator(String time) {
  char json[256];
  JsonDocument doc;
  doc["time"] = time;
  doc["value"] = random(210, 226);
  serializeJson(doc, json);
  return object_t(json);
}

#include "RelayManager.h"
#include <ArduinoJson.h>

void RelayManager::initializeRelays() {
  pinMode(relaiSatu, OUTPUT);
  pinMode(relaiDua, OUTPUT);
  pinMode(relaiTiga, OUTPUT);
  pinMode(relaiEmpat, OUTPUT);
  pinMode(relaiLima, OUTPUT);
  pinMode(relaiEnam, OUTPUT);
  pinMode(relaiTujuh, OUTPUT);
  pinMode(relaiDelapan, OUTPUT);
}

void RelayManager::initializeState(RealtimeDatabaseResult RTDB) {
  JsonDocument doc;
  deserializeJson(doc, RTDB.to<const char *>());

  digitalWrite(relaiSatu, bool(doc["room-a"]["lamp"]));
  digitalWrite(relaiDua, bool(doc["room-a"]["socket"]));
  digitalWrite(relaiTiga, bool(doc["room-b"]["lamp"]));
  digitalWrite(relaiEmpat, bool(doc["room-b"]["socket"]));
  digitalWrite(relaiLima, bool(doc["room-c"]["lamp"]));
  digitalWrite(relaiEnam, bool(doc["room-c"]["socket"]));
  digitalWrite(relaiTujuh, bool(doc["room-d"]["lamp"]));
  digitalWrite(relaiDelapan, bool(doc["room-d"]["socket"]));

  return;
}

void RelayManager::handleRelayUpdate(AsyncResult &aResult) {
  if (!aResult.available())
    return;

  RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
  if (RTDB.isStream()) {
    Serial.println("----------------------------");
    Firebase.printf("task: %s\n", aResult.uid().c_str());
    Firebase.printf("event: %s\n", RTDB.event().c_str());
    Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
    Firebase.printf("data: %s\n", RTDB.to<const char *>());
    Firebase.printf("type: %d\n", RTDB.type());

    const String path = RTDB.dataPath();
    if (path == "/") {
      initializeState(RTDB);
    }

    // put your handler here
    if (path == "/room-a/lamp") {
      digitalWrite(relaiSatu, RTDB.to<bool>());
    } else if (path == "/room-a/socket") {
      digitalWrite(relaiDua, RTDB.to<bool>());
    } else if (path == "/room-b/lamp") {
      digitalWrite(relaiTiga, RTDB.to<bool>());
    } else if (path == "/room-b/socket") {
      digitalWrite(relaiEmpat, RTDB.to<bool>());
    } else if (path == "/room-c/lamp") {
      digitalWrite(relaiLima, RTDB.to<bool>());
    } else if (path == "/room-c/socket") {
      digitalWrite(relaiEnam, RTDB.to<bool>());
    } else if (path == "/room-d/lamp") {
      digitalWrite(relaiTujuh, RTDB.to<bool>());
    } else if (path == "/room-d/socket") {
      digitalWrite(relaiDelapan, RTDB.to<bool>());
    }
  } else {
    Serial.println("----------------------------");
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(),
                    aResult.c_str());
  }

  Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
}

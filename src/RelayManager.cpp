#include "RelayManager.h"
#include <ArduinoJson.h>

unsigned long RelayManager::buzzerBlinkMs = 0;
bool RelayManager::isBuzzerBlinkActive = false;

void RelayManager::activateRelays() {
  pinMode(buzzer, OUTPUT);
  pinMode(relaiPompa, OUTPUT);

  pinMode(relaiSatu, OUTPUT);
  pinMode(relaiDua, OUTPUT);
  pinMode(relaiTiga, OUTPUT);
  pinMode(relaiEmpat, OUTPUT);
  pinMode(relaiLima, OUTPUT);
  pinMode(relaiEnam, OUTPUT);
  pinMode(relaiTujuh, OUTPUT);
  pinMode(relaiDelapan, OUTPUT);
}

void RelayManager::initializeRelayState(RealtimeDatabaseResult RTDB) {
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
}

void RelayManager::handleRelayUpdate(RealtimeDatabaseResult RTDB) {
  const String path = RTDB.dataPath();

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
}

void RelayManager::flameDetectedCallback() {
  // activate buzzer and water pump
  isBuzzerBlinkActive = true;
  digitalWrite(relaiPompa, HIGH);

  // turn off all the lamp and socket
  turnOffLampsAndSockets();
}

void RelayManager::flameClearedCallback() {
  // turn off buzzer and water pump
  isBuzzerBlinkActive = false;
  digitalWrite(relaiPompa, LOW);

  // TODO: re-initialize lamp, maybe
}

void RelayManager::smokeDetectedCallback() {
  // activate buzzer blink
  // digitalWrite(buzzer, HIGH);
  isBuzzerBlinkActive = true;
}

void RelayManager::smokeClearedCallback() {
  // turn off buzzer blink
  // digitalWrite(buzzer, LOW);
  isBuzzerBlinkActive = false;
}

void RelayManager::buzzerBlink() {
  static bool buzzerState = LOW; // Track the current state of the buzzer

  if (isBuzzerBlinkActive) {
    // Check if it's time to toggle the buzzer state
    if (millis() - buzzerBlinkMs >= 500) {
      buzzerBlinkMs = millis();   // Reset the timer
      buzzerState = !buzzerState; // Toggle the buzzer state
      digitalWrite(buzzer, buzzerState);
    }
  } else {
    // Ensure the buzzer is off when blinking is inactive
    digitalWrite(buzzer, LOW);
  }
}

void RelayManager::turnOffLampsAndSockets() {
  digitalWrite(relaiSatu, LOW);
  digitalWrite(relaiDua, LOW);
  digitalWrite(relaiTiga, LOW);
  digitalWrite(relaiEmpat, LOW);
  digitalWrite(relaiLima, LOW);
  digitalWrite(relaiEnam, LOW);
  digitalWrite(relaiTujuh, LOW);
  digitalWrite(relaiDelapan, LOW);
}

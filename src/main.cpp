#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Andini"
#define WIFI_PASSWORD "07121516"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAXzgcLtZMn4cGxqIS-cWq03SuFxdqtgLc"

// Insert RTDB URLefine the RTDB URL */
// #define DATABASE_URL "smart-grid-c2630-default-rtdb.asia-southeast1.firebasedatabase.app" 
#define DATABASE_URL "https://smart-grid-c2630-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

const int relaiSatu = 2;
const char* path = "/LED_STATUS"; // Change this to your database path

// This function will be called when there's a change in the Firebase database
void streamCallback(FirebaseStream data) {
  Serial.printf("stream path, %s\n", data.streamPath().c_str());
  printResult(data); //see addons/RTDBHelper.h
  
  // Only process if we have actual data
  if (data.dataType() == "boolean" || data.dataType() == "int") {
    bool ledStatus;

    if (data.dataType() == "boolean") {
      ledStatus = data.boolData();
    } else {
      ledStatus = (data.intData() == 1);
    }

    digitalWrite(relaiSatu, ledStatus ? HIGH : LOW);
    Serial.printf("LED is %s\n", ledStatus ? "ON" : "OFF");
  }

  Serial.println("-------------------");
}

// This function will be called if the stream times out
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Stream timeout, resume streaming...");
  }
  if (!Firebase.RTDB.beginStream(&fbdo, path)) {
    Serial.printf("stream begin error, %s\n\n", fbdo.errorReason().c_str());
  }
}

void setup(){
  Serial.begin(115200);

  pinMode(relaiSatu, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to Wi-Fi\n");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set up Firebase stream
  if (!Firebase.RTDB.beginStream(&fbdo, path)) {
    Serial.printf("stream begin error, %s\n\n", fbdo.errorReason().c_str());
  }

  // Set up the stream callback
  Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
}

void loop() {
  // The loop can be empty as we're using callbacks
  // You can add other code here if needed
  
  if (Firebase.ready() && signupOK) {
    // Your other code here if needed
  }

  delay(100); // Small delay to prevent watchdog timer issues
}

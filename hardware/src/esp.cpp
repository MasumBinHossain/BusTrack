#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

// WiFi Credentials
const char* ssid = "RAIKO";
const char* password = "1234567780";

// Google Script URL
const String sheet_url = "https://script.google.com/macros/s/AKfycbywrxIWuGwz4qfL9UVJ7ixZGNnCpFYB4mn4wHbyK1vIsg7wyHrosCZbN9Tvwmyx-KhN/exec";

// Placeholder for RFID UID
String rfidUID = "Waiting for data...";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    if (Serial.available()) {
        String incomingData = Serial.readStringUntil('\n');
        if (incomingData.startsWith("RFID:")) {
            std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
            client->setInsecure();
            rfidUID = incomingData.substring(5);
            String url = sheet_url + rfidUID;
            url.trim();
            Serial.println(url);

            HTTPClient https;
            Serial.print(F("[HTTPS] begin...\n"));
            if (https.begin(*client, (String)url)){
              Serial.print(F("[HTTPS] GET...\n"));
              int httpCode = https.GET();
              if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                // file found at server
                delay(2000);
              }
              else 
              {Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());}
            }
            https.end();
            delay(1000);
         

        }
        else {
          Serial.printf("[HTTPS} Unable to connect\n");
        }
    }
}

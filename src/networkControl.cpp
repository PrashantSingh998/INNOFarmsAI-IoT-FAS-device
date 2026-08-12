// networkControl.cpp

#include "configControl.h"
#include "deviceControl.h"
#include "bleControl.h"
#include "mqttControl.h"
#include "networkControl.h"
#include "otaControl.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>   // ✅ ESP8266 ONLY

float WIFI_RSSI = 0.0;

bool connect_to_wifi(const char* ssid, const char* password, bool recconnect = false) {
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  Serial.println(" Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
   Serial.print("PASSWORD: ");
  Serial.println(password);
  unsigned long timeout;
  
 

  WiFi.mode(WIFI_STA);           // Station mode
  // WiFi.begin(ssid, password);
  WiFi.begin("Airtel_INNOFARMS AI AGRITECH_4G", "Innofarm1008");

  unsigned long startAttemptTime = millis();
  if (recconnect){
    timeout = 60000;  // 60 seconds timeout
  }else {
    timeout = 15000;  // 30 seconds timeout
  }
 

  // Wait until connected or timeout
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    WIFI_RSSI = WiFi.RSSI();
    
    return true;
  } else {
    Serial.println("\n❌ Failed to connect to WiFi!");
    if (recconnect){
      Serial.println("\n❌ Restarting esp32 as it Failed to connect to WiFi!");
      delay(500);
      ESP.restart();
      
    }
    return false;
  }
}


bool checkWiFiConnection() {
  return (WiFi.status() == WL_CONNECTED);
}
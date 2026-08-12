#ifndef NETWORK_CONTROL_H
#define NETWORK_CONTROL_H

#include <Arduino.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

bool connect_to_wifi(const char* ssid, const char* password, bool recconnect);
bool checkWiFiConnection();

#endif // NETWORK_CONTROL_H

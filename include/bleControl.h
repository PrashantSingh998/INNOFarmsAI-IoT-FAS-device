#ifndef BLE_CONTROL_H
#define BLE_CONTROL_H

#include <Arduino.h>

#ifdef ESP8266
  #include <ESP8266WebServer.h>
  using WebServer_t = ESP8266WebServer;
#else
  #include <WebServer.h>
  using WebServer_t = WebServer;
#endif

// BLE control API (adjust if your bleControl.cpp expects different names)
void handleNotFound();
void handleGetStatus();
void handlecredentialsconfig(); 
void initroutes();
void httploop();
void handlecalibrationConfig();

#endif // BLE_CONTROL_H

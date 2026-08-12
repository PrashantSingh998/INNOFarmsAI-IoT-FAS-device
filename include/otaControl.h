#ifndef OTA_CONTROL_H
#define OTA_CONTROL_H

#include <Arduino.h>
#include <ArduinoOTA.h>


#include <WiFiClientSecure.h>
extern WiFiClientSecure wifiClientSecureOTA;



// Function declarations
void checkAndUpdateFirmware(const String &versionUrl,const char * currentVersion  );

void handleOTA();

#endif // OTA_CONTROL_H

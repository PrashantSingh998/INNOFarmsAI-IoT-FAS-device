#include "configControl.h"
#include "deviceControl.h"
#include "bleControl.h"
#include "mqttControl.h"
#include "networkControl.h"
#include "otaControl.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// ===== ESP8266 ONLY =====
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

// ================= OTA CHECK =================
void checkAndUpdateFirmware(const String &versionUrl, const char* currentVersion) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected");
    return;
  }

    WiFiClientSecure client;
    client.setInsecure();   HTTPClient http;

  Serial.println("🔍 Checking firmware version...");

  http.setTimeout(15000);

  if (!http.begin(client, versionUrl)) {
    Serial.println("❌ HTTP Begin Failed");
    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {

    String payload = http.getString();

    StaticJsonDocument<256> json;
    DeserializationError err = deserializeJson(json, payload);

    if (err) {
      Serial.println("❌ JSON Parse Failed");
      http.end();
      return;
    }

    String latest = json["version"].as<String>();
    String binUrl = json["bin_url"].as<String>();

    Serial.print("📦 Current Version: ");
    Serial.println(currentVersion);

    Serial.print("🆕 Latest Version: ");
    Serial.println(latest);

    http.end();

    if (latest.length() && latest != currentVersion) {

      Serial.println("🚀 New Firmware Found → Starting OTA");

      t_httpUpdate_return ret = ESPhttpUpdate.update(client, binUrl);

      switch (ret) {

        case HTTP_UPDATE_FAILED:
          Serial.printf("❌ OTA Failed (%d): %s\n",
                        ESPhttpUpdate.getLastError(),
                        ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("✔ No Updates Available");
          break;

        case HTTP_UPDATE_OK:
          Serial.println("✅ OTA Success → Rebooting");
          break;
      }

    } else {
      Serial.println("✔ Already Latest Version");
    }

  } else {
    Serial.printf("❌ HTTP Error Code: %d\n", httpCode);
    http.end();
  }
}

// ================= HANDLE OTA PLACEHOLDER =================
void handleOTA() {
  // Optional if using ArduinoOTA or periodic OTA logic
}

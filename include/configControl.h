#ifndef CONFIG_CONTROL_H
#define CONFIG_CONTROL_H

#include <Arduino.h>
#include <ArduinoJson.h>

// // ================== RUNTIME CONFIG (GLOBAL) ==================
// extern const char* WIFI_SSID;
// extern const char* WIFI_PASSWORD;

// extern const char* MQTT_SUBTOPIC;
// extern const char* MQTT_PUBTOPIC;
// extern const char* MQTT_RESPONSETOPIC;
// extern const char* KEEPALIVE_TOPIC;

// extern const char* MQTT_BROKER;
// extern int MQTT_PORT;
// extern const char* MQTT_USERNAME;
// extern const char* MQTT_PASSWORD;

// extern int TIME_OFFSET;
// extern int PUBLISH_INTERVAL;
// extern int KEEPALIVE_INTERVAL;
// extern int RACK_ID;

// extern bool shouldRestart;
// extern bool configMode;

// // ================== EEPROM ==================
// bool initEEPROM();
// void loadConfigFromEEPROM();
// void saveFarmIdToEEPROM(int farmId);
// void savePublishIntervalToEEPROM(int minutes);
// void saveKeepaliveTopicToEEPROM(const char* topic);


// ================== FILE SYSTEM ==================
bool initFileSystem();
bool fileExists(const char* path);
DynamicJsonDocument readJsonFile(const char* path);
bool updateJsonFile(const char* path, JsonDocument& doc);
void loadAllConfigFromFS();
void printSystemState();

bool updateJsonKey(const char* filePath, const char* key, const char* value);
bool updateConfig(
    String ssid, 
    String pass, 
    String subtop, 
    String pubtop, 
    String keepalive,
    int farmId,
    String infotop
    ) ;
bool saveFanChange();
#endif // CONFIG_CONTROL_H

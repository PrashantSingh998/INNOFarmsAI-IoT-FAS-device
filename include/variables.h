#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

// central config symbols (declared in configControl.cpp)
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

extern const char* MQTT_SUBTOPIC;
extern const char* MQTT_PUBTOPIC;
extern const char* MQTT_KEEPALIVETOPIC;
extern const char* MQTT_RESPONSETOPIC;
extern const char* MQTT_INFOTOPIC;

extern const char* MQTT_BROKER;
extern int MQTT_PORT;
extern const char* MQTT_USERNAME;
extern const char* MQTT_PASSWORD;
extern int FARM_ID;
extern int KEEPALIVE_INTERVAL; 
extern int RACK_ID; 
extern int TIME_OFFSET;

extern const char* DEVICE_ID;
extern const char* DEVICE_NAMES[];


extern String Motor_state;
extern String humi_state;
extern String fan1_state;
extern String fan2_state;
extern String fan3_state;
extern String fan4_state;
extern String fan5_state;
extern String ro_state;


extern int MODE;
extern int ON_TIME;
extern int OFF_TIME;


extern const char* Device_Version ;


extern bool shouldRestart;
extern bool configMode;
extern float PUBLISH_INTERVAL_MS;
extern float WIFI_RSSI;


extern bool statechangeFlag;
#endif // VARIABLES_H

#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


// MQTT control API used by other modules
extern PubSubClient client;
void initMqtt(const char* MQTT_BROKER, int MQTT_PORT);
void reconnect(const char* MQTT_SUBTOPIC, const char* MQTT_USERNAME, const char* mqttPassword);
bool isMqttConnected();
void mqttLoop();                    // runs PubSubClient loop

bool publish(const char* topic, const String& payload);
void publishState(bool keepAlive);  // publish current state (with optional keepalive topic)

String generateUUID();
bool publishDeviceInfo();

// Do NOT define getKeepaliveInterval() here — it's provided by configControl.cpp

#endif // MQTT_CONTROL_H

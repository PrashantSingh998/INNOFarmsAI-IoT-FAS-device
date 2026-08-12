#include "configControl.h"
#include "deviceControl.h"
#include "bleControl.h"
#include "mqttControl.h"
#include "networkControl.h"
#include "otaControl.h"
#include <ArduinoJson.h>
#include "variables.h"
#include <ESP8266mDNS.h>


WebServer_t server(80);


void initroutes() {
    if (!MDNS.begin(DEVICE_ID)) {   // Set the hostname to "esp32.local"
        Serial.println("Error setting up MDNS responder!");
        while(1) {
        delay(1000);
        }
    }
    Serial.println("mDNS responder started");
    server.on("/status", HTTP_GET, handleGetStatus);
    server.on("/config/credentials", HTTP_POST, handlecredentialsconfig);
    server.on("/config/calibration", HTTP_POST, handlecalibrationConfig);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP Server Started");
}


void handleGetStatus() {   // this is only to get the status of the devices 

    DynamicJsonDocument doc(1024);

    doc["wifi_ssid"] = WIFI_SSID;
    doc["wifi_password"] = WIFI_PASSWORD;   
    doc["mqtt_sub"] = MQTT_SUBTOPIC;
    doc["mqtt_pub"] = MQTT_PUBTOPIC;
    doc["mqtt_keep"] = MQTT_KEEPALIVETOPIC;

    doc["mode"] = MODE;
    doc["on_minutes"] = ON_TIME;
    doc["off_minutes"] = OFF_TIME;

    // ===== Button State Object =====
    JsonObject button = doc.createNestedObject("button_state");

    button["Motor"] = Motor_state;
    button["fan1"] = fan1_state;
    button["fan2"] = fan2_state;
    button["fan3"] = fan3_state;
    button["fan4"] = fan4_state;
    button["fan5"] = fan5_state;
    button["humi"] = humi_state;
    button["ro"] = ro_state;

    String response;
    serializeJson(doc, response);

    server.send(200, "application/json", response);

    Serial.println("Sent status response via HTTP: " + response);
}

void handlecredentialsconfig() {  // this is for wifi pass and farm id only 

    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }

    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "application/json", "{\"error\":\"Bad JSON\"}");
        return;
    }

    String ssid  = doc["WIFI_SSID"] | "";
    String pass  = doc["WIFI_PASSWORD"] | "";
    int farmId   = doc["FARM_ID"] | 0;

    // Check if at least one field is provided
    if (ssid == "" && pass == "" && farmId <= 0) {
        server.send(400, "application/json",
                    "{\"error\":\"No valid fields provided\"}");
        return;
    }

    bool result = updateConfig(ssid, pass, "", "", "", farmId , "");

    if (!result) {
        server.send(500, "application/json",
                    "{\"error\":\"Failed to save config\"}");
        return;
    }
    server.send(200, "application/json",
                "{\"msg\":\"Credentials Updated. Restarting...\"}");

    delay(1000);
    ESP.restart();
}

void handlecalibrationConfig() {      // this is for new schefuled and change mode (0 or 1)

    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body received\"}");
        return;
    }

    String body = server.arg("plain");

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    bool updated = false;

    if (doc.containsKey("on_minutes") || doc.containsKey("off_minutes")) {      // update scheduled

        int onMin  = doc["on_minutes"]  | ON_TIME;
        int offMin = doc["off_minutes"] | OFF_TIME;

        bool res1 = updateJsonKey("/config/scheduled.json", "on_t", String(onMin).c_str());
        bool res2 = updateJsonKey("/config/scheduled.json", "off_t", String(offMin).c_str());

        Serial.println("ON update result: " + String(res1));
        Serial.println("OFF update result: " + String(res2));

        ON_TIME = onMin;
        OFF_TIME = offMin;

        updated = true;
    }

    if (doc.containsKey("mode")) {      //update mode 0 , 1 

        int mode = doc["mode"] | MODE;

        if (mode != 0 && mode != 1) {
            server.send(400, "application/json", "{\"error\":\"Mode must be 0 or 1\"}");
            return;
        }
        
        bool res3 = updateJsonKey("/config/scheduled.json", "mode", String(mode).c_str());

        Serial.println("MODE update result: " + String(res3));
        MODE = mode;
        updated = true;
    }

    if (!updated) {
        server.send(400, "application/json", "{\"error\":\"No valid fields provided\"}");
        return;
    }
    
    server.send(200, "application/json", "{\"msg\":\"Calibration Updated. Restarting...\"}");
    delay(1000);
    ESP.restart();
}


void handleNotFound() {
  server.send(404, "text/plain", "404: Route Not Found");
}


void httploop() {
    server.handleClient();
}   
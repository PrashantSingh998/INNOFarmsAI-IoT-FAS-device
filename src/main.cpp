// // src/main.cpp
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "configControl.h"
#include "deviceControl.h"
#include "mqttControl.h"
#include "networkControl.h"
#include "otaControl.h"
#include "bleControl.h"
#include "variables.h"
#include <LittleFS.h>
#include <ESP8266mDNS.h>



unsigned long previousMillis = 0;
unsigned long keepaliveMillis = 0;
unsigned long statusCheckMillis = 0;
unsigned long fanCycleMillis = 0;

unsigned long previousfanMillis = 0;
bool fanStateflag = false;
unsigned long lastWiFiAnimation = 0;

unsigned long lastSensorUpdate = 0;
unsigned long lastWaterQualityRotation = 0;

void updateWiFiConnectingAnimation() {
    const unsigned long now = millis();
    if (now - lastWiFiAnimation >= 350) {
        lastWiFiAnimation = now;
        drawWiFiAnimation(wifiAnimationFrame++);
    }
}

void setup() {
  Serial.begin(115200);
//   delay(5000);
  Serial.println("\n<<<<<< ESP12e Display Contriller >>>>>>\n");
  initFileSystem();
  loadAllConfigFromFS();
  printSystemState();
  initDevices();
  showStartupLogo();
  connect_to_wifi(WIFI_SSID, WIFI_PASSWORD,false);
  Serial.println("✅ Device Initialization Complete");  
  // OTA check (optional)
    checkAndUpdateFirmware(String("https://device-ota-bucket.s3.ap-south-1.amazonaws.com/Climate_Control/IFFNC1200000001/meta.json"), Device_Version);
    initroutes();
    initMqtt(MQTT_BROKER, MQTT_PORT);
    reconnect(MQTT_SUBTOPIC, MQTT_USERNAME, MQTT_PASSWORD);
    publishDeviceInfo();
    Serial.println("Setup complete.\n");
}

void loop() {
  unsigned long currentMillis = millis();

  // Periodic WiFi/MQTT checks
    if (currentMillis - previousMillis >= 15000UL) {
        previousMillis = currentMillis;
        if (!checkWiFiConnection()) {
            if (dashboardVisible || !reconnectScreenVisible) {
                showWiFiConnectingScreen(true);
            }
            Serial.println("WiFi disconnected, reconnecting...");
            connect_to_wifi(WIFI_SSID, WIFI_PASSWORD,true);
            reconnect(MQTT_SUBTOPIC, MQTT_USERNAME, MQTT_PASSWORD);
        }
        else{
            if (!dashboardVisible) {
                reconnectScreenVisible = false;
                dashboardVisible = true;
                internetAvailable = isMqttConnected();
                drawMainDashboard();
                drawHeaderConnectivityIcons();
            }
            return;
        }
    }

    if (!dashboardVisible) {
        updateWiFiConnectingAnimation();
        delay(10);
        return;
    }

    if (currentMillis - lastWaterQualityRotation >= 5000UL) {
        lastWaterQualityRotation = currentMillis;
        rotateWaterQualityDisplay();
    }

    if (currentMillis - lastSensorUpdate >= 120000) {
        lastSensorUpdate = currentMillis;
        // updateDemoSensorValues();
        updateSensorValuesOnly(); // Only values are refreshed.
        updateWaterQualityContext();
        drawControllerStatusLine();
    }

    if(!isMqttConnected()){
        reconnect(MQTT_SUBTOPIC, MQTT_USERNAME, MQTT_PASSWORD);
    }
    if(!client.connected()){
        reconnect(MQTT_SUBTOPIC, MQTT_USERNAME, MQTT_PASSWORD);
    }

    if((status_duration !=0) && currentMillis - status_millis >= status_duration){
        updateControllerMessage("Controllers are idle");
        status_millis = 0;
        status_duration = 0;
    }
    // MQTT message processing
    mqttLoop();
    httploop();
    MDNS.update();
    if (millis() - keepaliveMillis >= 60000UL) {
        keepaliveMillis = millis();
        publishState(true);
    }

  }








// #include <Arduino.h>
// #include <LittleFS.h>
 
// // -------- PRINT FILE CONTENT --------
// void printFile(const char* path) {
 
//   Serial.print("\n📄 Reading: ");
//   Serial.println(path);
 
//   File file = LittleFS.open(path, "r");
 
//   if (!file) {
//     Serial.println("❌ Failed to open file");
//     return;
//   }
 
//   while (file.available()) {
//     Serial.write(file.read());
//   }
 
//   file.close();
//   Serial.println("\n✅ Read Done");
// }
 
// // -------- CREATE FILE + PRINT --------
// void createFile(const char* path, const char* data) {
 
//   if (!LittleFS.exists(path)) {
 
//     Serial.print("\nCreating: ");
//     Serial.println(path);
 
//     File file = LittleFS.open(path, "w");
 
//     if (!file) {
//       Serial.println("❌ File Create Failed");
//       return;
//     }
 
//     file.print(data);
//     file.close();
 
//     Serial.println("✅ File Saved");
 
//   } else {
//     Serial.print("\nAlready Exists: ");
//     Serial.println(path);
//   }
 
//   // Print file content after save / exists
//   printFile(path);
// }
 
// // -------- INIT CONFIG FS --------
// void initConfigFS() {
 
//   Serial.println("\n===== INIT CONFIG FS =====");
 
//   // ---- Mount FS ----
//   if (!LittleFS.begin()) {
 
//     Serial.println("Mount Failed → Formatting");
 
//     if (!LittleFS.format()) {
//       Serial.println("❌ Format Failed");
//       return;
//     }
 
//     if (!LittleFS.begin()) {
//       Serial.println("❌ Mount Failed After Format");
//       return;
//     }
//   }
 
//   Serial.println("✅ FS Mounted");
 
//   // -------- CREDENTIALS --------
//   createFile(
//     "/config/credentials.json",
//     "{\n"
//     "  \"ssid\": \"Airtel_INNOFARMS AI AGRITECH_4G\",\n"
//     "  \"password\": \"Innofarm1008\",\n"
//     "  \"broker\": \"mqtt.innofarms.ai\",\n"
//     "  \"mqtt_password\": \"innofarmweb2024\",\n"
//     "  \"username\": \"webusers\",\n"
//     "  \"farm_id\": \"120\"\n"
//     "}\n"
//   );
 
//   // -------- TOPICS --------
//   createFile(
//     "/config/topics.json",
//     "{\n"
//     "  \"pub\": \"actuator\",\n"
//     "  \"sub\": \"SSub\",\n"
//     "  \"kp\": \"actuator_keepalive\",\n"
//     "  \"info\": \"Info\"\n"
//     "}\n"
// );
//   // -------- STATE --------
//   createFile(
//     "/config/state.json",
//     "{\n"
//     "  \"Motor\": \"off\",\n"
//     "  \"fan5\": \"off\",\n"
//     "  \"fan1\": \"off\",\n"
//     "  \"humi\": \"off\",\n"
//     "  \"fan2\": \"off\",\n"
//     "  \"fan3\": \"off\",\n"
//     "  \"ro\": \"off\",\n"
//     "  \"fan4\": \"off\"\n"
//     "}\n"
//   );
 
//   // -------- SCHEDULED --------
//   createFile(
//     "/config/scheduled.json",
//     "{\n"
//     "  \"mode\": 1,\n"
//     "  \"on_t\": 10,\n"
//     "  \"off_t\": 10\n"
//     "}\n"
//   );
 
//   Serial.println("\n===== CONFIG FS READY =====");
// }
 
// // -------- SETUP --------
// void setup() {
 
//   Serial.begin(115200);
//   delay(1000);
 
//   initConfigFS();
// }
 
// // -------- LOOP --------
// void loop() {
// }











// #include <Arduino.h>
// #include <LittleFS.h>
 
// void setup() {
//   Serial.begin(115200);
//   delay(1000);
 
//   Serial.println("\n===== ESP8266 FILE SYSTEM CLEANER =====");
 
//   // Mount LittleFS
//   if (!LittleFS.begin()) {
//     Serial.println("❌ LittleFS Mount Failed!");
//     Serial.println("⚠️ Trying to format...");
 
//     if (LittleFS.format()) {
//       Serial.println("✅ Format Success!");
     
//       // Try mounting again
//       if (LittleFS.begin()) {
//         Serial.println("✅ LittleFS Mounted After Format");
//       } else {
//         Serial.println("❌ Mount Failed Even After Format");
//       }
 
//     } else {
//       Serial.println("❌ Format Failed");
//     }
 
//   } else {
//     Serial.println("✅ LittleFS Mounted");
 
//     // ---- Clean / Format Existing FS ----
//     Serial.println("🧹 Cleaning File System...");
 
//     LittleFS.end();   // Unmount first
 
//     if (LittleFS.format()) {
//       Serial.println("✅ File System Cleaned Successfully");
//     } else {
//       Serial.println("❌ File System Clean Failed");
//     }
 
//     // Remount after cleaning
//     if (LittleFS.begin()) {
//       Serial.println("✅ LittleFS Remounted");
//     } else {
//       Serial.println("❌ Remount Failed");
//     }
//   }
 
//   Serial.println("===== DONE =====");
// }
 
// void loop() {
//   // Nothing needed
// }

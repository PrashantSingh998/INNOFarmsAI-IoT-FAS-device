#include "configControl.h"
#include "variables.h"
#include <LittleFS.h>
#include "deviceControl.h"
#include "Arduino.h"
#include <ArduinoJson.h>

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

const char* MQTT_BROKER = "";
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

const char* MQTT_SUBTOPIC = "";
const char* MQTT_PUBTOPIC = "";
const char* MQTT_INFOTOPIC = "";
const char* MQTT_KEEPALIVETOPIC = "";

String Motor_state = "";
String fan1_state = "";
String fan2_state = "";
String fan3_state = "";
String fan4_state = "";
String fan5_state = "";
String humi_state = "";
String ro_state   = "";

int MODE = 0;
int ON_TIME = 0;
int OFF_TIME = 0;

int FARM_ID = 0;
int MQTT_PORT = 1883;
bool shouldRestart = false;
bool configMode = false;


DynamicJsonDocument credDoc(1024);
DynamicJsonDocument sensDoc(1024);
DynamicJsonDocument topicDoc(1024);

const char* Device_Version = "1.0.0";

// ================= FILE SYSTEM =================
bool initFileSystem() {
  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS Mount Failed");
    return false;
  }
  Serial.println("✅ LittleFS Mounted");
  return true;
}

bool fileExists(const char* path) {
  return LittleFS.exists(path);
}

DynamicJsonDocument readJsonFile(const char* path) {
  DynamicJsonDocument doc(512);
  if (!LittleFS.exists(path)) return doc;

  File file = LittleFS.open(path, "r");
  deserializeJson(doc, file);
  file.close();
  return doc;
}

bool updateJsonFile(const char* path, JsonDocument& doc) {
  File file = LittleFS.open(path, "w");
  if (!file) return false;
  serializeJsonPretty(doc, file);
  file.close();
  return true;
}

void loadAllConfigFromFS() {
  

  // ===== CREDENTIALS =====
  if (fileExists("/config/credentials.json")) {

   
    File f = LittleFS.open("/config/credentials.json", "r");
    if (f) {
      DeserializationError error = deserializeJson(credDoc, f);
      if (error) {
        Serial.print("❌ Failed to parse credentials.json: ");
        Serial.println(error.c_str());
        return;
      }
      
      f.close();

      if (credDoc.isNull()) {
       
        return;
      }
      WIFI_SSID = credDoc["ssid"].as<const char*>();
      
      WIFI_PASSWORD = credDoc["password"].as<const char*>();

      MQTT_BROKER = credDoc["broker"].as<const char*>();
      MQTT_USERNAME = credDoc["username"].as<const char*>();
      MQTT_PASSWORD = credDoc["mqtt_password"].as<const char*>();
      

      FARM_ID = credDoc["farm_id"].as<int>();
      

    }
  }
  if (!LittleFS.exists("/sensor")) {
    LittleFS.mkdir("/sensor");
  }

  if (fileExists("/sensor/lastValues.json")){
    File f = LittleFS.open("/sensor/lastValues.json", "r");
    if (f) {
      DeserializationError error = deserializeJson(sensDoc, f);
      if (error) {
        Serial.print("❌ Failed to parse lastValues.json: ");
        Serial.println(error.c_str());
        return;
      }

      f.close();

      if (sensDoc.isNull()) return;
      sensors.temperature = sensDoc["Etemp"].as<float>();
      sensors.humidity = sensDoc["humi"].as<float>();
      sensors.co2 = sensDoc["co2"].as<float>();
      sensors.waterLevel = sensDoc["wl"].as<float>();
      sensors.ph11 = sensDoc["ph11"].as<float>();
      sensors.ec11 = sensDoc["ec11"].as<float>();
      sensors.waterTemperature11 = sensDoc["wt11"].as<float>();
      sensors.ph12 = sensDoc["ph12"].as<float>();
      sensors.ec12 = sensDoc["ec12"].as<float>();
      sensors.waterTemperature12 = sensDoc["wt12"].as<float>();
      sensors.ph21 = sensDoc["ph21"].as<float>();
      sensors.ec21 = sensDoc["ec21"].as<float>();
      sensors.waterTemperature21 = sensDoc["wt21"].as<float>();
      sensors.ph22 = sensDoc["ph22"].as<float>();
      sensors.ec22 = sensDoc["ec22"].as<float>();
      sensors.waterTemperature22  = sensDoc["wt22"].as<float>();
    }
    else {
      sensDoc.clear();

      sensDoc["Etemp"] = 0.0;
      sensDoc["humi"] = 0.0;
      sensDoc["co2"] = 0.0;
      sensDoc["wl"] = 0.0;

      sensDoc["ph11"] = 0.0;
      sensDoc["ec11"] = 0.0;
      sensDoc["wt11"] = 0.0;

      sensDoc["ph12"] = 0.0;
      sensDoc["ec12"] = 0.0;
      sensDoc["wt12"] = 0.0;

      sensDoc["ph21"] = 0.0;
      sensDoc["ec21"] = 0.0;
      sensDoc["wt21"] = 0.0;

      sensDoc["ph22"] = 0.0;
      sensDoc["ec22"] = 0.0;
      sensDoc["wt22"] = 0.0;

      File f = LittleFS.open("/sensor/lastValues.json", "w");

      if (!f)
      {
          Serial.println("Failed to create lastValues.json");
          return;
      }

      if (serializeJsonPretty(sensDoc, f) == 0)
      {
          Serial.println("Failed to write lastValues.json");
          f.close();
          return;
      }

      f.close();
      Serial.println("Created /sensor/lastValues.json with default values");
    }
  } else {
    sensDoc.clear();

    sensDoc["Etemp"] = 0.0;
    sensDoc["humi"] = 0.0;
    sensDoc["co2"] = 0.0;
    sensDoc["wl"] = 0.0;

    sensDoc["ph11"] = 0.0;
    sensDoc["ec11"] = 0.0;
    sensDoc["wt11"] = 0.0;

    sensDoc["ph12"] = 0.0;
    sensDoc["ec12"] = 0.0;
    sensDoc["wt12"] = 0.0;

    sensDoc["ph21"] = 0.0;
    sensDoc["ec21"] = 0.0;
    sensDoc["wt21"] = 0.0;

    sensDoc["ph22"] = 0.0;
    sensDoc["ec22"] = 0.0;
    sensDoc["wt22"] = 0.0;

    File f = LittleFS.open("/sensor/lastValues.json", "w");

    if (!f)
    {
        Serial.println("Failed to create lastValues.json");
        return;
    }

    if (serializeJsonPretty(sensDoc, f) == 0)
    {
        Serial.println("Failed to write lastValues.json");
        f.close();
        return;
    }

    f.close();
    Serial.println("Created /sensor/lastValues.json with default values");
  }
  // ===== TOPICS =====
  if (fileExists("/config/topics.json")) {
   
    File f = LittleFS.open("/config/topics.json", "r");
   
    if (f) {
      deserializeJson(topicDoc, f);
      f.close();

      static String pubTopicStr;
      static String subTopicStr;
      static String keepTopicStr;
      static String infoTopicStr;
     

      pubTopicStr =
        String("farm/") + String(FARM_ID) + "/" + topicDoc["pub"].as<String>();

      subTopicStr =
        String("farm/") + String(FARM_ID) + "/" + topicDoc["sub"].as<String>();

      keepTopicStr =
        String("farm/") + String(FARM_ID) + "/" + topicDoc["kp"].as<String>();
      
      infoTopicStr =
        String("farm/") + String(FARM_ID) + "/" + topicDoc["info"].as<String>();
      
      MQTT_PUBTOPIC = pubTopicStr.c_str();
      MQTT_SUBTOPIC = subTopicStr.c_str();
      MQTT_KEEPALIVETOPIC = keepTopicStr.c_str();
      MQTT_INFOTOPIC = infoTopicStr.c_str();
    }

  }

  Serial.println("✅ All Config Loaded From FS");
}

void printSystemState() {

  Serial.println("\n========== SYSTEM STATE ==========");

  Serial.println("\n--- WiFi ---");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  Serial.println("\n--- MQTT ---");
  Serial.print("Broker: ");
  Serial.println(MQTT_BROKER);

  Serial.print("Username: ");
  Serial.println(MQTT_USERNAME );

  Serial.print("MQTT_PASSWORD: ");
  Serial.println(MQTT_PASSWORD );

  Serial.print("Sub Topic: ");
  Serial.println(MQTT_SUBTOPIC);

  Serial.print("Pub Topic: ");
  Serial.println(MQTT_PUBTOPIC);

  Serial.print("Keepalive Topic: ");
  Serial.println(MQTT_KEEPALIVETOPIC);

  Serial.print("Info Topic: ");
  Serial.println(MQTT_INFOTOPIC);


  Serial.println("\n==================================\n");
  Serial.println();
  Serial.println("========== SENSOR VALUES ==========");

  Serial.print("Temperature: ");
  Serial.println(sensors.temperature, 2);

  Serial.print("Humidity: ");
  Serial.println(sensors.humidity, 2);

  Serial.print("CO2: ");
  Serial.println(sensors.co2);

  Serial.print("Water Level: ");
  Serial.println(sensors.waterLevel, 2);

  Serial.println("------ Zone 1 Unit 1 ------");

  Serial.print("pH 11: ");
  Serial.println(sensors.ph11, 2);

  Serial.print("EC 11: ");
  Serial.println(sensors.ec11, 2);

  Serial.print("Water Temperature 11: ");
  Serial.println(sensors.waterTemperature11, 2);

  Serial.println("------ Zone 1 Unit 2 ------");

  Serial.print("pH 12: ");
  Serial.println(sensors.ph12, 2);

  Serial.print("EC 12: ");
  Serial.println(sensors.ec12, 2);

  Serial.print("Water Temperature 12: ");
  Serial.println(sensors.waterTemperature12, 2);

  Serial.println("------ Zone 2 Unit 1 ------");

  Serial.print("pH 21: ");
  Serial.println(sensors.ph21, 2);

  Serial.print("EC 21: ");
  Serial.println(sensors.ec21, 2);

  Serial.print("Water Temperature 21: ");
  Serial.println(sensors.waterTemperature21, 2);

  Serial.println("------ Zone 2 Unit 2 ------");

  Serial.print("pH 22: ");
  Serial.println(sensors.ph22, 2);

  Serial.print("EC 22: ");
  Serial.println(sensors.ec22, 2);

  Serial.print("Water Temperature 22: ");
  Serial.println(sensors.waterTemperature22, 2);

  Serial.println("===================================");

}


bool updateJsonKey(const char* filePath, const char* key, const char* value){

  if (!LittleFS.exists(filePath)) {
      Serial.println("File not found");
      return false;
  }
  File file = LittleFS.open(filePath, "r");
    if (!file) {
      Serial.println("Failed to open file for reading in updateJsonKey");
      return false;
    }
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
      Serial.println("JSON parse failed in updateJsonKey");
      Serial.println(error.c_str());
      return false;
  }

  // Update key
  if (String(key) == "on_minutes" || String(key) == "off_minutes" || String(key) == "mode" || String(key) == "farm_id") {
    doc[key] = atoi(value);
  } else {
  doc[key] = value;
  }

  // Write back to file
  file = LittleFS.open(filePath, "w");
  if (!file) {
      Serial.println("Failed to open file for writing");
      return false;
  }

  serializeJson(doc, file);
  file.close();

  return true;
}


bool updateConfig(
    String ssid, 
    String pass, 
    String subtop, 
    String pubtop, 
    String keepalive,
    int farmId,
    String infotop
    ) {
 ///-------------------------------------------------------------------------------------
  if (ssid.length() > 0 || pass.length() > 0 || farmId > 0) {
    if(!LittleFS.exists("/config/credentials.json")) {
        Serial.println("File not found");
        return false;
    }
    File file = LittleFS.open("/config/credentials.json", "r");
      if (!file) {
        Serial.println("Failed to open file for reading in updateJsonKey");
        return false;
      }
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("JSON parse failed in updateJsonKey");
        Serial.println(error.c_str());
        return false;
    }

    // Update key
    if (ssid.length() > 0) {    
      doc["ssid"] = ssid;
    }
    if (pass.length() > 0) {      
      doc["password"] = pass;
    }
    if (farmId > 0) {
      doc["farm_id"] = farmId;
    }

    // Write back to file
    file = LittleFS.open("/config/credentials.json", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }

    serializeJson(doc, file);
    file.close();
    delay(200);
  }

  ///-------------------------------------------------------------------------------------
  if (subtop.length() > 0 || pubtop.length() > 0 || keepalive.length() > 0 || infotop.length() > 0) {
    if (!LittleFS.exists("/config/topics.json")) {
        Serial.println("File not found");
        return false;
    }
    File file = LittleFS.open("/config/topics.json", "r");
      if (!file) {
        Serial.println("Failed to open file for reading in updateJsonKey");
        return false;
      }
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("JSON parse failed in updateJsonKey");
        Serial.println(error.c_str());
        return false;
    }

    // Update key
    if (subtop.length() > 0) {    
      doc["sub"] = subtop;
    }
    if (pubtop.length() > 0) {      
      doc["pub"] = pubtop;
    }
    if (keepalive.length() > 0) {
      doc["kp"] = keepalive;
    }
    
    if (infotop.length() > 0) {
      doc["info"] = infotop;
    }

    // Write back to file
    file = LittleFS.open("/config/topics.json", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }

    serializeJson(doc, file);
    file.close();
    delay(200);
    
  }
  return true;
}

  


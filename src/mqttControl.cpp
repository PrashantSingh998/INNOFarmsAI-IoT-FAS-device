#include "mqttControl.h"
#include "configControl.h"
#include "deviceControl.h"
#include "networkControl.h"
#include "variables.h"

#include <PubSubClient.h>
#include <ArduinoJson.h>


#include <ESP8266WiFi.h>


// single WiFiClient + PubSubClient instance
WiFiClient espClient;
PubSubClient client(espClient);





// Device id (keeps compatibility with your other code)
const char* DEVICE_ID = "IFFAS1210000001";

// snapshot string (status.json without Packet_Id)
static String lastStatusSnapshot = "";

// Forward declaration for callback required by PubSubClient
void mqttCallback(char* topic, byte* payload, unsigned int length);

// ---------------- UUID v4 generator (ESP8266-safe)
String generateUUID() {
  // Seed pseudo-random with chip id + micros
#ifdef ESP8266
  uint32_t seed = ESP.getChipId() ^ (uint32_t)micros();
#else
  uint32_t seed = (uint32_t)ESP.getEfuseMac() ^ (uint32_t)micros();
#endif
  randomSeed(seed);

  uint8_t uuid[16];
  for (int i = 0; i < 16; ++i) uuid[i] = (uint8_t)random(0, 256);

  // set version 4
  uuid[6] = (uuid[6] & 0x0F) | 0x40;
  // set variant
  uuid[8] = (uuid[8] & 0x3F) | 0x80;

  char buf[37];
  sprintf(buf,
          "%02x%02x%02x%02x-"
          "%02x%02x-"
          "%02x%02x-"
          "%02x%02x-"
          "%02x%02x%02x%02x%02x%02x",
          uuid[0], uuid[1], uuid[2], uuid[3],
          uuid[4], uuid[5],
          uuid[6], uuid[7],
          uuid[8], uuid[9],
          uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
  return String(buf);
}

// ---------------- init / reconnect / loop ----------------
void initMqtt(const char* MQTT_BROKER, int MQTT_PORT) {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setBufferSize(1024);
  // client.setServer("broker.emqx.io", MQTT_PORT);
  client.setCallback(mqttCallback);
  // use keepalive value from configControl (single definition)
  client.setKeepAlive(40);
  Serial.println("✅ MQTT initialized");
}

void reconnect(const char* MQTT_SUBTOPIC, const char* MQTT_USERNAME, const char* mqttPassword) {
  if (!checkWiFiConnection()) {
    Serial.println("⚠️ WiFi not connected, skipping MQTT reconnect");
    return;
  }

  int attempts = 0;
  const int maxAttempts = 3;

  while (!client.connected() && attempts < maxAttempts) {
    Serial.println("🔌 Attempting MQTT connection...");
    String deviceMac = generateUUID();
     Serial.println("UUID ");

    if (client.connect(deviceMac.c_str(), MQTT_USERNAME, mqttPassword)) {
      // optional indicator pin (some boards use GPIO2)
      pinMode(2, OUTPUT);
      digitalWrite(2, HIGH);
      

      String farmTopicBase = String("farm/") + String(FARM_ID) + "/";
      String sensorTopic = farmTopicBase + "sensor";
      String actuatorTopic = farmTopicBase + "actuator";
      String fertigationTopic = farmTopicBase + "fertigation";
      String subTopic = farmTopicBase + "SSub";
      String irrigationTopic = farmTopicBase + "IIrrigation";
      String irrigationTopicAlt = farmTopicBase + "Irrigation";

      client.subscribe(sensorTopic.c_str());
      client.subscribe(actuatorTopic.c_str());
      client.subscribe(fertigationTopic.c_str());
      client.subscribe(subTopic.c_str());
      client.subscribe(irrigationTopic.c_str());
      client.subscribe(irrigationTopicAlt.c_str());

      Serial.println("✅ Connected to MQTT broker");
      Serial.println("📬 Subscribed to: " + sensorTopic);
      Serial.println("📬 Subscribed to: " + actuatorTopic);
      Serial.println("📬 Subscribed to: " + fertigationTopic);
      Serial.println("📬 Subscribed to: " + subTopic);
      Serial.println("📬 Subscribed to: " + irrigationTopic);
      Serial.println("📬 Subscribed to: " + irrigationTopicAlt);
      return;
    } else {
      attempts++;
      Serial.printf("❌ MQTT connect failed, rc=%d (Attempt %d/%d)\n",
                    client.state(), attempts, maxAttempts);
      pinMode(2, OUTPUT);
      digitalWrite(2, LOW);
      delay(3000);
    }
  }

  if (!client.connected()) {
    Serial.println("⚠️ MQTT connection failed after max attempts");
    ESP.restart();
  }
}

bool isMqttConnected() {
  return client.connected();
}

void mqttLoop() {
  if (client.connected()) client.loop();
}

// ---------------- MQTT callback (handles incoming commands) ----------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.println("\n📩 MQTT RX");
  Serial.println("Topic: " + String(topic));
  Serial.println("Payload: " + msg);
  String fullTopic = String(topic);

  int firstSlash = fullTopic.indexOf('/');
  int secondSlash = fullTopic.indexOf('/', firstSlash + 1);

  String rootTopic = "";
  String farmId = "";
  String topicType = "";

  if (firstSlash != -1 && secondSlash != -1) {
    rootTopic = fullTopic.substring(0, firstSlash);
    farmId = fullTopic.substring(firstSlash + 1, secondSlash);
    topicType = fullTopic.substring(secondSlash + 1);
  }

  Serial.println("Root: " + rootTopic);
  Serial.println("Farm ID: " + farmId);
  Serial.println("Type: " + topicType);

  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, msg)) {
    Serial.println("❌ JSON parse error");
    return;
  }
  if((topicType== "sensor" )&& (doc.containsKey("DN"))) {         // In this we cover  THC , NSD && WLD
    String DN = doc["DN"] | "";
    if (DN == "THC") {
      if (!doc.containsKey("Etemp") || !doc.containsKey("Humidity") || !doc.containsKey("CO2")){
        Serial.println("missing keys in THC  Payload");
        return ;
      }
      sensors.temperature = doc["Etemp"];
      sensors.humidity = doc["Humidity"];
      sensors.co2 = doc["CO2"];
      Serial.println("latest rading of THC :- Temp: "+String(sensors.temperature)+", Humidity:" + String(sensors.humidity)+ ", Co2: "+ String(sensors.co2));
      bool result1 =  updateJsonKey("/sensor/lastValues.json", "Etemp", String(sensors.temperature).c_str());
      bool result2 =  updateJsonKey("/sensor/lastValues.json", "humi", String(sensors.humidity).c_str());
      bool result3 =  updateJsonKey("/sensor/lastValues.json", "co2", String(sensors.co2).c_str());
      Serial.println("THC reading in file get updated : " + String(result1)+ String(result2)+ String(result3));
      delay(200);
      // update_values_in_Fs("THC",0,0);
      return;

    }
    if (DN == "WLD") {
      if (!doc.containsKey("WaterLevel") ){
        Serial.println("missing keys in WLD  Payload");
        return ;
      }
      sensors.waterLevel = doc["WaterLevel"];
      Serial.println("latest rading of WLD :- waterLevel: "+String(sensors.waterLevel));
      bool result3 =  updateJsonKey("/sensor/lastValues.json", "wl", String(sensors.waterLevel).c_str());
      Serial.println("WLD reading in file get updated : " +  String(result3));
      delay(200);
      return;
    // update_values_in_Fs("WLD",0,0);  
    }
    if (DN == "NSD") {
        if (!doc.containsKey("PH Sensor") || !doc.containsKey("EC Sensor") || !doc.containsKey("Water Temperature Sensor") || !doc.containsKey("rack") || !doc.containsKey("shelve")){
        Serial.println("missing keys in NSD  Payload");
        return ;
      }
      int rack = doc["rack"];
      int shelve = doc["shelve"];
      String cases = doc["cases"];
      if ( cases == "last") {
        if ((rack ==1) && (shelve ==1)){
          sensors.ph11 = doc["PH Sensor"];
          sensors.ec11 = doc["EC Sensor"];
          sensors.waterTemperature11 = doc["Water Temperature Sensor"];
          Serial.println("latest rading of NSD for Rack "+String(rack)+ "Shelve:- "+ String(shelve)+ "and  pH: "+String(sensors.ph11)+", eC:" + String(sensors.ec11)+ ", waterTemperature: "+ String(sensors.waterTemperature11));
          bool result1 =  updateJsonKey("/sensor/lastValues.json", "ph11", String(sensors.ph11).c_str());
          bool result2 =  updateJsonKey("/sensor/lastValues.json", "ec11", String(sensors.ec11).c_str());
          bool result3 =  updateJsonKey("/sensor/lastValues.json", "wt11", String(sensors.waterTemperature11).c_str());
          Serial.println("NSD reading in file get updated : " + String(result1)+ String(result2)+ String(result3));
          delay(200);
          return;

        } if ((rack ==1) && (shelve ==2)){
          sensors.ph12 = doc["PH Sensor"];
          sensors.ec12 = doc["EC Sensor"];
          sensors.waterTemperature12 = doc["Water Temperature Sensor"];
          Serial.println("latest rading of NSD for Rack "+String(rack)+ "Shelve:- "+ String(shelve)+ "and  pH: "+String(sensors.ph12)+", eC:" + String(sensors.ec12)+ ", waterTemperature: "+ String(sensors.waterTemperature12));
          bool result1 =  updateJsonKey("/sensor/lastValues.json", "ph12", String(sensors.ph12).c_str());
          bool result2 =  updateJsonKey("/sensor/lastValues.json", "ec12", String(sensors.ec12).c_str());
          bool result3 =  updateJsonKey("/sensor/lastValues.json", "wt12", String(sensors.waterTemperature12).c_str());
          Serial.println("NSD reading in file get updated : " + String(result1)+ String(result2)+ String(result3));
          delay(200);
          return;
          // update_values_in_Fs("THC",0,0);
        }
        if ((rack ==2) && (shelve ==1)){
          sensors.ph21 = doc["PH Sensor"];
          sensors.ec21 = doc["EC Sensor"];
          sensors.waterTemperature21 = doc["Water Temperature Sensor"];
          Serial.println("latest rading of NSD for Rack "+String(rack)+ "Shelve:- "+ String(shelve)+ "and  pH: "+String(sensors.ph21)+", eC:" + String(sensors.ec21)+ ", waterTemperature: "+ String(sensors.waterTemperature21));
          bool result1 =  updateJsonKey("/sensor/lastValues.json", "ph21", String(sensors.ph21).c_str());
          bool result2 =  updateJsonKey("/sensor/lastValues.json", "ec21", String(sensors.ec21).c_str());
          bool result3 =  updateJsonKey("/sensor/lastValues.json", "wt21", String(sensors.waterTemperature21).c_str());
          Serial.println("NSD reading in file get updated : " + String(result1)+ String(result2)+ String(result3));
          delay(200);
          return;
          // update_values_in_Fs("THC",0,0);

        }if ((rack ==2) && (shelve ==2)){
          sensors.ph22 = doc["PH Sensor"];
          sensors.ec22 = doc["EC Sensor"];
          sensors.waterTemperature22 = doc["Water Temperature Sensor"];
          Serial.println("latest rading of NSD for Rack "+String(rack)+ "Shelve:- "+ String(shelve)+ "and  pH: "+String(sensors.ph22)+", eC:" + String(sensors.ec22)+ ", waterTemperature: "+ String(sensors.waterTemperature22));
          bool result1 =  updateJsonKey("/sensor/lastValues.json", "ph22", String(sensors.ph22).c_str());
          bool result2 =  updateJsonKey("/sensor/lastValues.json", "ec22", String(sensors.ec22).c_str());
          bool result3 =  updateJsonKey("/sensor/lastValues.json", "wt22", String(sensors.waterTemperature22).c_str());
          Serial.println("NSD reading in file get updated : " + String(result1)+ String(result2)+ String(result3));
          delay(200);
          return;
          // update_values_in_Fs("THC",0,0);
        }
      }
    }
    else {
      Serial.println("message is not for uses in the topic Sensor");
    }
    
  } 
  if((topicType== "SSub" )&& (doc.containsKey("DN"))) {      // In this we cover  FNC and AFC
    String DN = doc["DN"] | "";
    if (DN == "FNC") {
      String cmdd = doc["cmd"];
      String devicetype = doc["device_type"];
      String status = doc["status"];
      if (cmdd == "changestate"){
        if ((devicetype == "fan1") && (status == "on")) {
          updateControllerMessage("Fogger is running at Unit 1");
          Serial.println("fogger is runnig at unit 1 ");
        }
        if ((devicetype == "fan2") && (status == "on")) {
          updateControllerMessage("Fogger is running at Unit 2");
          Serial.println("fogger is runnig at unit 2 ");
        }
        if ((devicetype == "fan1") && (status == "off")) {
          updateControllerMessage("Fogger is idle at Unit 1");
          Serial.println("fogger is idle at unit 1 ");
          status_millis = millis();
          status_duration =  3000;

        }
        if ((devicetype == "fan2") && (status == "off")){
          updateControllerMessage("Fogger is idle at Unit 2");
          Serial.println("fogger is idle at unit 2 ");
          status_millis = millis();
          status_duration =  3000;
        }
      }
    }
  }
  if(((topicType== "IIrrigation" ) || (topicType == "Irrigation")) && (doc.containsKey("DN"))) {      // In this we cover  IDC 
    String DN = doc["DN"] | "";
    if (DN == "IDC") {
      String cmmd = doc["cmd"] | "";
      if (cmmd == "drain"){
        updateControllerMessage("Refill is running");
        Serial.println("Refill is running");
        float dur = doc["duration"] | 1;
        status_millis = millis();
        status_duration = dur*60 *1000; 
      }
      if (cmmd == "irrigate"){
        float dur = doc["duration"] | 1;
        int shelf = doc["shelf_id"];
        int rack = doc["rack_id"];
        if ((rack ==1) && (shelf == 1)){
         updateControllerMessage("Irrigation: Unit 1, Zone 1");
         Serial.println("Irrigation is running at unit 1 zone 1");
          status_duration = dur*60 *1000; 
          status_millis = millis();
        }
        if ((rack ==1) && (shelf == 2)){
          updateControllerMessage("Irrigation: Unit 1, Zone 2");
          Serial.println("Irrigation is running at unit 1 zone 2");
          status_duration = dur*60 *1000;
          status_millis = millis(); 
        }
        if ((rack ==2) && (shelf == 1)){
          updateControllerMessage("Irrigation: Unit 2, Zone 1");
          Serial.println("Irrigation is running at unit 2 zone 1");
          status_duration = dur*60 *1000; 
          status_millis = millis();
        }
        if ((rack ==2) && (shelf == 2)){
          updateControllerMessage("Irrigation: Unit 2, Zone 2");
          Serial.println("Irrigation is running at unit 2 zone 2");
          status_duration = dur*60 *1000; 
          status_millis = millis();
        }
      }
    }
  }
  if((topicType== "fertigation" )&& (doc.containsKey("DN"))) {      // In this we cover  FUD 
    String cmds = doc["cmd"];
    if (cmds == "change_limits"){
      updateControllerMessage("Fertigation is running");
      status_duration = 30*60 *1000;
      status_millis = millis();
    } 
  }
  String cmd = doc["cmd"] | "";
  String DNNNN = doc["DN"] | "";

  // ---------- server_detail ----------
  if( (cmd == "server_detail")&&(topicType== "SSub" )&& (DNNNN =="FAS")) {
    String ssid   = doc["WIFI_SSID"]              | "";
    String pass   = doc["WIFI_PASSWORD"]          | "";
    String subtop = doc["MQTT_SUBTOPIC"]          | "";
    String pubtop = doc["MQTT_PUBTOPIC"]          | "";
    String keepalive = doc["MQTT_KEEPALIVETOPIC"]    | "";
    String infotop = doc["MQTT_INFOTOPIC"]    | "";
    int farmId    = doc["FARM_ID"]                | 0;
   
    updateConfig(ssid, pass, subtop, pubtop, keepalive, farmId, infotop);

    delay(300);
    ESP.restart();
  }


  // ---------- keepalive ----------
  if (cmd == "keepalive") {
    publishState(false);
  }
}


// ---------------- publish helpers ----------------
bool publish(const char* topic, const String& payload) {
  if (!client.connected()) {
    Serial.println("⚠️ MQTT not connected. Publish failed.");
    return false;
  }
  bool ok = client.publish(topic, payload.c_str());
  if (ok) {
    Serial.println("📤 Publish OK -> " + String(topic));
  } else {
    Serial.println("❌ Publish Failed -> " + String(topic));
  }
  return ok;
}

void publishState(bool keepAlive = false)
{
    DynamicJsonDocument doc(256
    );

    doc["DN"] = "FAS";
    doc["Device_Id"] = DEVICE_ID;
    doc["Mode"] = MODE;
    doc["WIFI_RSSI"] = WIFI_RSSI;

    if (keepAlive)
    {
        doc["cmd"] = "keepalive";
        doc["Packet_Id"] = generateUUID();
    }
    else
    {
        doc["cmd"] = "state";
    }

    String payload;
    serializeJson(doc, payload);

    Serial.print("Publishing payload: ");
    Serial.println(payload);

    if (keepAlive)
    {
        publish(MQTT_KEEPALIVETOPIC, payload);
    }
    else
    {
        publish(MQTT_PUBTOPIC, payload);
    }
}

// ---------------- optional device-info publish ----------------
bool publishDeviceInfo() {
  DynamicJsonDocument doc(256);
  doc["MAC"] = WiFi.macAddress();
  doc["IP"] = WiFi.localIP().toString();
#ifdef ESP8266
  doc["CHIP_No"] = String((uint32_t)ESP.getChipId());
#else
  doc["CHIP_No"] = String((uint32_t)ESP.getEfuseMac());
#endif
  doc["DN"] = "FAS";
  doc["version"] = Device_Version;
  doc["Device_Id"] = DEVICE_ID;

  String out;
  serializeJson(doc, out);

  Serial.println("\n📋 Publishing device info:");
  Serial.print("Topic: ");
  Serial.println(MQTT_INFOTOPIC); 
  Serial.println(out);

  if (MQTT_PUBTOPIC == nullptr || strlen(MQTT_PUBTOPIC) == 0) {
    Serial.println("⚠️ MQTT_PUBTOPIC not set - cannot publish device info");
    return false;
  }

  return publish(MQTT_INFOTOPIC, out);
}

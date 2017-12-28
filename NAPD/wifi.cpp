#include "wifi.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "led.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
char mqttTopicMaster[25];
char mqttTopicSlave[25];

String Get_macID (void);

void wifiConfig() {
  static int _state = 0;
  if(_state == 0) {
    Serial.println("Start smart config!!!");
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
    _state = 1;
  }
  else if(_state == 1) {
    if(!WiFi.smartConfigDone()) {
      delay(20);
    }
    else {
      Serial.println("Smart Config done");
      WiFi.stopSmartConfig();
      _state = 0;
    }
  }
}

void wifiConnect() {
  static int _state = 2;
  if(_state == 2) {
    Serial.println("Start connecting WiFi");
    WiFi.begin();
    WiFi.printDiag(Serial);
    Serial.println();
    _state = 3;
  }
  else if(_state == 3) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(20);
    }
    else {
      Serial.println("WiFi connected");
      _state = 4;
    }
  }
  else if(_state == 4) {
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected");
      _state = 2;
    }
  }
}

void mqttConnect() {
  static int _state = 5;
  if(WiFi.status() == WL_CONNECTED) {
    if(_state == 5) {
      Serial.println("Setup MQTT");
      mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
      mqttClient.setCallback(mqttCallback);
      _state = 6;
    }
    else if(_state == 6) {
      if(!mqttClient.connect("hgnhgn")) {
        delay(20);
      }
      else {
        Serial.println("MQTT Broker connected");
        String mqtt_topic_rx = Get_macID() + "/master";
        mqtt_topic_rx.toCharArray(mqttTopicMaster, 24);
        mqttClient.subscribe(mqttTopicMaster);
        Serial.println("MQTT Subscribed");
        _state = 7;
      }
    }
    else if(_state == 7) {
      if(!mqttClient.connected()) {
        Serial.println("MQTT Broker disconnected");
        _state = 5;
      }
      else {
        mqttClient.loop();
      }
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += String((char)payload[i]);
  }
  DynamicJsonBuffer _jsonBuffer;
  JsonObject& root = _jsonBuffer.parseObject(msg);
  String func = root["FUNC"];
  String data = root["DATA"];
  String mac = root["MACID"];
  String pass = root["PASS"];
  if(pass != "8888") return;
  // if(mac != Get_macID()) return;
  String mqtt_topic_tx = Get_macID() + "/slave";
  mqtt_topic_tx.toCharArray(mqttTopicSlave, 24);
  if(func == "1") {
    ledPowerControl((uint8_t)(data.toInt()));
    mqttClient.publish(mqttTopicSlave, payload);
  }
}

String Get_macID (void)
{
  String _val;
  byte _mac[6];
  WiFi.macAddress(_mac);
  for (int i = 0; i < 6; i++)
  {
    if (_mac[i] < 0x10)
      _val += '0' + String(_mac[i], HEX);
    else _val += String(_mac[i], HEX);
  }
  return _val;
}


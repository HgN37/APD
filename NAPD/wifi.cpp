#include "wifi.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "led.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
char mqttTopicMaster[25];
char mqttTopicSlave[25];
String frameFunc = "";
String frameData = "";
String frameMAC = "";
String framePass = "";

String Get_macID (void);

int wifiConfig() {
  ledControl(LOW);
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
      delay(20);
      Serial.println("Smart Config done");
      _state = 2;
    }
  }
  else if(_state == 2) {
    _state = 0;
  }
  return _state;
}

int wifiConnect() {
  static int _state = 0;
  static uint32_t _led_delay = 0;
  if(_state == 0) {
    Serial.println("Start connecting WiFi");
    WiFi.begin();
    WiFi.printDiag(Serial);
    Serial.println();
    _state = 1;
  }
  else if(_state == 1) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(20);
    }
    else {
      Serial.println("WiFi connected");
      _state = 2;
    }
  }
  else if(_state == 2) {
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected");
      _state = 0;
    }
  }
  
  if(_state == 0) {
    ledControl(LOW);
  }
  else if(_state == 1) {
    if((millis() - _led_delay) > 500) {
      ledToggle();
      _led_delay = millis();
    }
  }
  else if(_state == 2) {
    ledControl(HIGH);
  }
  return _state;
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
        Serial.println("MQTT Topic Subscribed: " + mqtt_topic_rx);
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
  Serial.println("Receive from topic: " + String(topic));
  for (int i = 0; i < length; i++) {
    msg += String((char)payload[i]);
  }
  Serial.println("Msg: " + msg);
  DynamicJsonBuffer _jsonBuffer;
  JsonObject& root = _jsonBuffer.parseObject(msg);
  String func = root["FUNC"];
  String data = root["DATA"];
  String mac = root["MACID"];
  String pass = root["PASS"];
  frameFunc = func;
  frameData = data;
  frameMAC = mac;
  framePass = pass;
}

String mqttGetFunc() {
  return frameFunc;
}

String mqttGetData() {
  return frameData;
}

String mqttGetMAC() {
  return frameMAC;
}

String mqttGetPass() {
  return framePass;
}

void mqttClearFrame() {
  frameFunc = "";
  frameData = "";
  frameMAC = "";
  framePass = "";
}

void mqttPublish(String pJsonOut) {
  char _dataOut[100];
  pJsonOut.toCharArray(_dataOut, pJsonOut.length() + 1);
  String mqtt_topic_tx = Get_macID() + "/slave";
  mqtt_topic_tx.toCharArray(mqttTopicSlave, 24);
  mqttClient.publish(mqttTopicSlave, _dataOut);
  Serial.println("Send to " + mqtt_topic_tx + ": " + pJsonOut);
}

String mqttCreateJson(String func, String data) {
  String json;
  json = "{\"FUNC\":\"" + func + "\",\"DATA\":\"" + data + "\",\"MAC\":\"" + Get_macID() + "\",\"PASS\":\"8888\"}";
  return json;
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


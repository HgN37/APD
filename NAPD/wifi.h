#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include "defineAll.h"

int wifiConfig();
int wifiConnect();
void mqttConnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttClearFrame();
String mqttGetPass();
String mqttGetMAC();
String mqttGetData();
String mqttGetFunc();
void mqttPublish(String pJsonOut);
String mqttCreateJson(String func, String data);

#endif

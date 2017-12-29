#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include "defineAll.h"

int wifiConfig();
int wifiConnect();
void mqttConnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif

#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include "defineAll.h"

void wifiConfig();
void wifiConnect();
void mqttConnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif

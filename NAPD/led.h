#ifndef LED_H
#define LED_H

#include "Arduino.h"
#include "defineAll.h"

void ledInit();
void ledControl(uint8_t value);
void ledToggle();
void ledPowerControl(uint8_t value);

#endif

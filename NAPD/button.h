#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"
#include "defineAll.h"

void buttonInit();
bool buttonReadPressed();
uint8_t buttonReadAdc();

#endif

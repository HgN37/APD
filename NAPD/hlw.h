#ifndef HLW_H
#define HLW_H

#include "Arduino.h"
#include "defineAll.h"

void hlwInit();
void hlwUpdate();
float hlwGetVolage();
float hlwGetCurrent();
float hlwGetActivePower();
float hlwGetReactivePower();
float hlwGetApparentPower();


#endif

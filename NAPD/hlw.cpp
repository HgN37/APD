#include "hlw.h"
#include <HLW8012.h>
#include <EEPROM.h>

float current = 0;
float voltage = 0;
float activePower = 0;
float reactivePower = 0;
float aparentPower = 0;

HLW8012 hlw8012;

void hlwInit() {
  EEPROM.begin(50);
  uint32_t vCurrent = 0;
  uint32_t vVoltage = 0;
  uint32_t vPower = 0;
  for(int i = 0; i < 4; i++) {
    vCurrent <<= 8;
    vVoltage <<= 8;
    vPower <<= 8;
    vCurrent |= (uint32_t)EEPROM.read(0x10 + i);
    vVoltage |= (uint32_t)EEPROM.read(0x14 + i);
    vPower |= (uint32_t)EEPROM.read(0x18 + i);
  }
  hlw8012.begin(PIN_CF, PIN_CF1, PIN_SEL, CURRENT_MODE, false, 500000);
  hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
  hlw8012.setCurrentMultiplier((float)(vCurrent / 100.0));
  hlw8012.setVoltageMultiplier((float)(vVoltage / 100.0));
  hlw8012.setPowerMultiplier((float)(vPower / 100.0));
  EEPROM.end();
}

void hlwUpdate() {
  static uint32_t last_read = 0;
  if((millis() - last_read) > 2000) {
    voltage = (float)hlw8012.getVoltage();
    current = (float)hlw8012.getCurrent();
    activePower = (float)hlw8012.getActivePower();
    reactivePower = (float)hlw8012.getReactivePower();
    aparentPower = (float)hlw8012.getApparentPower();
    hlw8012.toggleMode();
    last_read = millis();
  }
}

float hlwGetVolage() {
  return voltage;
}

float hlwGetCurrent() {
  return current;
}

float hlwGetActivePower() {
  return activePower;
}

float hlwGetReactivePower() {
  return reactivePower;
}

float hlwGetApparentPower() {
  return aparentPower;
}


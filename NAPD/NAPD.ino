#include "led.h"
#include "button.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  ledInit();
  buttonInit();
  while(2 != wifiConfig()) delay(10);
  while(2 != wifiConnect()) delay(10);
}

void loop() {
}


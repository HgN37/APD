#include "led.h"
#include "button.h"
#include "wifi.h"

void setup() {
  ledInit();
  buttonInit();
  wifiConfig();
  wifiConnect();
}

void loop() {
  
}


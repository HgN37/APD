#include "button.h"

void buttonInit() {
  pinMode(PIN_BUTTON, INPUT);

  analogWriteRange(100);
}

bool buttonReadPressed() {
  static uint8_t vLastStatus = HIGH;
  static uint32_t vLastPressed = 0;
  static bool vPressed = false;
  uint8_t reading = digitalRead(PIN_CONFIG);
  if(reading == LOW) {
      if(reading != vLastStatus) {
          vLastPressed = millis();
      }
      else if((millis() - vLastPressed) > BUTTON_DELAY) {
          if(vPressed == false) {
              vPressed = true;
              vLastStatus = reading;
              return true;
          }
      }
  }
  else {
      vPressed = false;
  }
  vLastStatus = reading;
  return false;
}

uint8_t buttonReadAdc() {
  return (uint8_t)(analogRead(A0)*100/1024);
}


#include "led.h"

void ledInit() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  analogWriteRange(100);
}

void ledControl(uint8_t value) {
  digitalWrite(PIN_LED, value);
}

void ledToggle() {
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}

void ledPowerControl(uint8_t value) {
  if(value < LED_POWER_OFF_VALUE) {
    digitalWrite(PIN_RELAY, LOW); // tat led
  }
  else {
    digitalWrite(PIN_RELAY, HIGH);
    analogWrite(PIN_PWM, value);
  }
}

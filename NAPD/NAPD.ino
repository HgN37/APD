#include "led.h"
#include "button.h"
#include "wifi.h"
#include "hlw.h"

bool flag_config = true;
bool flag_adc_enable = true;
uint8_t pwm_value = 0;
uint8_t adc_value = 0;
uint8_t adc_new = 0;
int adc_diff;
String json = "";
String databig = "";

void setup() {
  Serial.begin(115200);
  ledInit();
  buttonInit();
  hlwInit();
}

void loop() {
  //! WiFi and MQTT connection
  if(false == flag_config) {
    if(2 == wifiConfig()) {
      flag_config = true;
    }
  }
  else {
    wifiConnect();
    mqttConnect();
  }
  if(true == buttonReadPressed()) {
    flag_config = !flag_config;
  }
  
  //!  Read ADC to control
  if(flag_adc_enable == true) {
    adc_new = buttonReadAdc();
    delay(10);
    if(adc_new != adc_value) {
      adc_value = adc_new;
      //Serial.println(adc_value);
    }
    pwm_value = adc_value;
    ledPowerControl(pwm_value);
  }
  else {
    adc_new = buttonReadAdc();
    delay(10);
    adc_diff = (int)adc_new - (int)adc_value;
    if(abs(adc_diff) > 20) {
      flag_adc_enable = true;
    }
  }

  //! Update HLW value
  hlwUpdate();

  //! Read MQTT CMD
  if(mqttGetPass() == "8888") {
    if(mqttGetFunc() == "0") {
      mqttPublish(mqttCreateJson("0", String(pwm_value)));
    }
    else if(mqttGetFunc() == "1") {
      pwm_value = (uint8_t)(mqttGetData().toInt());
      if(pwm_value > 100) pwm_value = 100;
      ledPowerControl(pwm_value);
      mqttPublish(mqttCreateJson("1", mqttGetData()));
      flag_adc_enable = false;
    }
    else if(mqttGetFunc() == "2") {
      databig = "{\"U\":\"";
      databig += String(hlwGetVolage()) + "\",";
      databig += "\"I\":\"";
      databig += String(hlwGetCurrent()) + "\",";
      databig += "\"P\":\"";
      databig += String(hlwGetActivePower()) + "\",";
      databig += "\"Q\":\"";
      databig += String(hlwGetReactivePower()) + "\",";
      databig += "\"S\":\"";
      databig += String(hlwGetApparentPower()) + "\"}";
      mqttPublish(mqttCreateJson("2", databig));
    }
    else {
      mqttPublish(mqttCreateJson("3", ""));
    }
  }

  //! Reset MQTT frame
  mqttClearFrame();
}


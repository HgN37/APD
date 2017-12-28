#include "task.h"
#include <HLW8012.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

/* CÁC ĐỊNH NGHĨA GIÁ TRỊ HẰNG */
const char* wifi_ssid = "BLACK";
const char* wifi_password = "yoursolution";
const char* mqtt_server = "iot.eclipse.org";
const int mqtt_port = 1883;
const char* mqtt_topic_rx = "APD/master";
const char* mqtt_topic_tx = "APD/slave";

/* CÁC BIẾN VÀ ĐỐI TƯỢNG TOÀN CỤC */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
HLW8012 hlw8012;

bool config_done = false;
bool config_start = false;
bool calib_done = false;
uint32_t wifi_start = 0;
int adc;
int wifi_status = 0;
float current = 0;
float voltage = 0;
float activePower = 0;
float reactivePower = 0;
float aparentPower = 0;

void taskSetup() {
  //! Thiết lập cổng giao tiếp nối tiếp phục vụ debug
  Serial.begin(115200);
  while(!Serial) yield();
  
  //! Thiết lập các chân ngõ ra
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BUTTON, INPUT);
  
  //! Thiết lập chân PWM
  analogWriteRange(100);
  
  //! Thiết lập HLW8012
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

void taskWiFiConfig() {
  static int _state = 0;
  if(_state == 0) {
    Serial.println("Start smart config!!!");
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
    _state = 1;
  }
  else if(_state == 1) {
    if(!WiFi.smartConfigDone()) {
      delay(20);
    }
    else {
      Serial.println("Smart Config done");
      _state = 2;
    }
  }
  wifi_status = _state;
}

void taskWiFiConnect() {
  static int _state = 2;
  if(_state == 2) {
    Serial.println("Start connecting WiFi");
    WiFi.begin();
    WiFi.printDiag(Serial);
    Serial.println();
    _state = 3;
  }
  else if(_state == 3) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(20);
    }
    else {
      Serial.println("WiFi connected");
      _state = 4;
    }
  }
  else if(_state == 4) {
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected");
      _state = 2;
    }
  }
  wifi_status = _state;
}

void taskMqttConnect() {
  static int _state = 5;
  if(WiFi.status() == WL_CONNECTED) {
    if(_state == 5) {
      Serial.println("Setup MQTT");
      mqttClient.setServer(mqtt_server, mqtt_port);
      mqttClient.setCallback(mqttCallback);
      _state = 6;
    }
    else if(_state == 6) {
      if(!mqttClient.connect("hgnhgn")) {
        delay(20);
      }
      else {
        Serial.println("MQTT Broker connected");
        mqttClient.subscribe(mqtt_topic_rx);
        Serial.println("MQTT Subscribed");
        _state = 7;
      }
    }
    else if(_state == 7) {
      if(!mqttClient.connected()) {
        Serial.println("MQTT Broker disconnected");
        _state = 5;
      }
      else {
        mqttClient.loop();
      }
    }
  }
  wifi_status = _state;
}

void taskMqttCallback() {
  
}

void taskHLW() {
  static uint32_t last_read = 0;
  if((millis() - last_read) > 2000) {
    voltage = (float)hlw8012.getVoltage();
    current = (float)hlw8012.getCurrent();
    activePower = (float)hlw8012.getActivePower();
    reactivePower = (float)hlw8012.getReactivePower();
    aparentPower = (float)hlw8012.getApparentPower();
    hlw8012.toggleMode();
    last_read = millis();
    // Send HLW value to read
    Serial.print("WiFi status   :"); Serial.println(wifi_status);
    Serial.print("Voltage       :"); Serial.println(voltage);
    Serial.print("Current       :"); Serial.println(current);
    Serial.print("Active Power  :"); Serial.println(activePower);
    Serial.print("Reactive Power:"); Serial.println(reactivePower);
    Serial.print("Aparent Power :"); Serial.println(aparentPower);
    Serial.println();
  }
}

/**********************************************************************
 * LOCAL FUNCTION
 *********************************************************************/
/* 
 * HÀM ĐỌC NÚT NHẤN
 */
bool buttonReadPressed() {
    static uint8_t vLastStatus = HIGH;
    static uint32_t vLastPressed = 0;
    static bool vPressed = false;
    uint8_t reading = digitalRead(PIN_CONFIG);
    if(reading == LOW) {
        if(reading != vLastStatus) {
            vLastPressed = millis();
        }
        else if((millis() - vLastPressed) > 150) {
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

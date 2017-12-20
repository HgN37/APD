#include <HLW8012.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

/* CONSTANT DEFINES */
const char* wifi_ssid = "BLACK";
const char* wifi_password = "yoursolution";
const char* mqtt_server = "iot.eclipse.org";
const int mqtt_port = 1883;
const char* mqtt_topic_rx = "HGN37RX";
const char* mqtt_topic_tx = "HGN37TX";

/* GPIO DEFINES */
#define PIN_LED    15
#define PIN_BUTTON 2
#define PIN_RELAY  4
#define PIN_PWM    5
#define PIN_SEL    12
#define PIN_CF     14
#define PIN_CF1    13

/* HLW8012 DEFINE */
#define CURRENT_MODE HIGH
#define VOLTAGE_MODE LOW
#define CURRENT_RESISTOR            (float)0.001
#define VOLTAGE_RESISTOR_UPSTREAM   ( 5 * 470000 )
#define VOLTAGE_RESISTOR_DOWNSTREAM ( 1000 )

/* GLOBAL OBJECTS/VARIABLES */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
HLW8012 hlw8012;
uint32_t last_read = 0;
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

/* SETUP FUNCTION
 *  - Setup gpio input/output.
 *  - Init Serial for debug.
 *  - Init HLW8012.
 *  - Connect to WiFi and MQTT broker.
 */
void setup() {
  //! Setup serial for debug
  Serial.begin(115200);
  while(!Serial) yield();
  
  //! Setup output pin
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); // Turn led off
  pinMode(PIN_BUTTON, INPUT);
  
  //! Setup PWM pin
  analogWriteRange(100);
  
  //! Setup HLW8012
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
  /*
  //! Setup WiFi
  Serial.println("\nStart smart config");
  WiFi.stopSmartConfig();
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  while(!WiFi.smartConfigDone()) {
    delay(100);
  }
  WiFi.stopSmartConfig();
  Serial.println("Smart Config done");
  WiFi.begin();
  WiFi.printDiag(Serial);
  Serial.println();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.println("WiFi connected");

  //! Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  while(!mqttClient.connect("hgnhgn")) yield();
  if(mqttClient.connected()) {
    Serial.println("Connected to MQTT server");
  }
  else {
    Serial.println("Fail to connect to server");
  }
  mqttClient.subscribe(mqtt_topic_rx);
  digitalWrite(PIN_LED, HIGH); // To inform that Init done
  */
}

/* LOOP FUNCTION
 *  - MQTT loop.
 *  - Read ADC to control PWM.
 */
void loop() {
  if(!mqttClient.connected()) {
    digitalWrite(PIN_LED, LOW);
    //! WiFi error
    if(WiFi.status() != WL_CONNECTED) {
      //! Not config yet
      if(config_done == false) {
        if(config_start == false) {
          Serial.println("\nStart smart config");
          WiFi.stopSmartConfig();
          WiFi.mode(WIFI_STA);
          WiFi.beginSmartConfig();
          config_start = true;
        }
        else {
          wifi_status = 1;
          if(WiFi.smartConfigDone()) {
            WiFi.stopSmartConfig();
            Serial.println("Smart Config done");
            config_done = true;
          }
        }
      }
      else {
        wifi_status = 2;
        if((millis() < 1000) | ((millis() - wifi_start) > 10000)) {
          WiFi.begin();
          WiFi.printDiag(Serial);
          wifi_start = millis();
        }
      }
    }
    //! MQTT error
    else {
      wifi_status = 3;
      if((millis() - wifi_start) > 2000) {
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(mqttCallback);
        mqttClient.connect("hgnhgn");
      }
    }
  }
  else {
    wifi_status = 4;
    digitalWrite(PIN_LED, HIGH); // To inform that Init done
    mqttClient.subscribe(mqtt_topic_rx);
    mqttClient.loop();
  }
  adc = (int)(analogRead(A0)*100/1024);
  analogWrite(PIN_PWM, adc);
  if(buttonReadPressed() == true) {
    digitalWrite(PIN_RELAY, !digitalRead(4));
    /*
    //! Calib
    if(calib_done == false) {
      Serial.println("Wait until stable");
      delay(5000);
      delay(5000);
      Serial.println("Start calib");
      hlw8012.getActivePower();
      hlw8012.setMode(MODE_CURRENT);
      delay(2000);
      hlw8012.getCurrent();
      hlw8012.setMode(MODE_VOLTAGE);
      delay(2000);
      hlw8012.getVoltage();
      hlw8012.expectedActivePower(257.0);
      hlw8012.expectedVoltage(226.0);
      hlw8012.expectedCurrent(1.172);
      Serial.println("Calib done, save to EEPROM");
      uint32_t xCurrent = (uint32_t)(hlw8012.getCurrentMultiplier() * 100.0);
      uint32_t xVoltage = (uint32_t)(hlw8012.getVoltageMultiplier() * 100.0);
      uint32_t xPower = (uint32_t)(hlw8012.getPowerMultiplier() * 100);
      EEPROM.begin(50);
      EEPROM.write(0x10, (uint8_t)(xCurrent >> 24));
      EEPROM.write(0x11, (uint8_t)(xCurrent >> 16));
      EEPROM.write(0x12, (uint8_t)(xCurrent >> 8));
      EEPROM.write(0x13, (uint8_t)(xCurrent >> 0));
      EEPROM.write(0x14, (uint8_t)(xVoltage >> 24));
      EEPROM.write(0x15, (uint8_t)(xVoltage >> 16));
      EEPROM.write(0x16, (uint8_t)(xVoltage >> 8));
      EEPROM.write(0x17, (uint8_t)(xVoltage >> 0));
      EEPROM.write(0x18, (uint8_t)(xPower >> 24));
      EEPROM.write(0x19, (uint8_t)(xPower >> 16));
      EEPROM.write(0x1A, (uint8_t)(xPower >> 8));
      EEPROM.write(0x1B, (uint8_t)(xPower >> 0));
      EEPROM.commit();
      EEPROM.end();
      calib_done = true;
    }*/
  }
  //! Read HLW
  if((millis() - last_read) > 2000) {
    voltage = (float)hlw8012.getVoltage();
    current = (float)hlw8012.getCurrent();
    activePower = (float)hlw8012.getActivePower();
    reactivePower = (float)hlw8012.getReactivePower();
    aparentPower = (float)hlw8012.getApparentPower();
    // This function switch between mode
    // Need about 2s to make sure the value is stable
    // So each time we get into the condition we only update 1 value, other values return the cache one.
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
  //! Just prevent watchdog timer reset
  yield();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += String((char)payload[i]);
  }
  Serial.print("Get msg: ");
  Serial.println(msg);
  String cmd = msg.substring(0,1);
  long data = msg.substring(1).toInt();
  Serial.print("Parse: cmd=");
  Serial.println(cmd);
  Serial.print("Parse: data=");
  Serial.println(data);
  if(cmd == "R") {
    if(data == 0) {
      digitalWrite(4, LOW);
    }
    else if(data == 100) {
      digitalWrite(4, HIGH);
    }
  }
  else if(cmd == "C") {
    analogWrite(5, (int)(analogRead(A0)*100/1024));
  }
  else if(cmd == "P") {
    analogWrite(5, data);
  }
}

bool buttonReadPressed() {
    static uint8_t vLastStatus = HIGH;
    static uint32_t vLastPressed = 0;
    static bool vPressed = false;
    uint8_t reading = digitalRead(PIN_BUTTON);
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




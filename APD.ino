#include <HLW8012.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

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
int adc;
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
  hlw8012.begin(PIN_CF, PIN_CF1, PIN_SEL, CURRENT_MODE, false, 500000);
  hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);

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
  mqttClient.publish(mqtt_topic_tx, "reseted and connected");
  digitalWrite(PIN_LED, HIGH); // To inform that Init done

  //! Calib
  hlw8012.getActivePower();
  hlw8012.setMode(MODE_CURRENT);
  delay(2000);
  hlw8012.getCurrent();
  hlw8012.setMode(MODE_VOLTAGE);
  delay(2000);
  hlw8012.getVoltage();
  hlw8012.expectedActivePower(260.0);
  hlw8012.expectedVoltage(222.5);
  hlw8012.expectedCurrent(1.207);
}

/* LOOP FUNCTION
 *  - MQTT loop.
 *  - Read ADC to control PWM.
 */
void loop() {
  mqttClient.loop();
  adc = (int)(analogRead(A0)*100/1024);
  analogWrite(PIN_PWM, adc);
  if(buttonReadPressed() == true) {
    digitalWrite(PIN_RELAY, !digitalRead(4));
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




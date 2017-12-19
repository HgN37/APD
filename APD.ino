#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* Constant Define */
const char* wifi_ssid = "BLACK";
const char* wifi_password = "yoursolution";
const char* mqtt_server = "iot.eclipse.org";
const int mqtt_port = 1883;
const char* mqtt_topic_rx = "HGN37RX";
const char* mqtt_topic_tx = "HGN37TX";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

int x;

void setup() {
  //! Setup serial for debug
  Serial.begin(115200);
  while(!Serial) yield();
  //! Setup output pin
  pinMode(4, OUTPUT);
  //! Setup PWM pin
  analogWriteRange(100);
 
  //! Setup WiFi
  WiFi.begin(wifi_ssid, wifi_password);
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
}

void loop() {
  while(1) {
    mqttClient.loop();
    yield();
  }
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
      digitalWrite(4, HIGH);
    }
    else if(data == 100) {
      digitalWrite(4, LOW);
    }
  }
  else if(cmd == "C") {
    analogWrite(5, (int)(analogRead(A0)*100/1024));
  }
  else if(cmd == "P") {
    analogWrite(5, data);
  }
}


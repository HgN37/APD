#ifndef ALL_H
#define ALL_H

/* CÁC ĐỊNH NGHĨA CHÂN */
#define PIN_CONFIG 0
#define PIN_LED    15
#define PIN_BUTTON 2
#define PIN_RELAY  4
#define PIN_PWM    5
#define PIN_SEL    12
#define PIN_CF     14
#define PIN_CF1    13

/* CÁC ĐỊNH NGHĨA CHO HLW8012 */
#define CURRENT_MODE HIGH
#define VOLTAGE_MODE LOW
#define CURRENT_RESISTOR            (float)0.001
#define VOLTAGE_RESISTOR_UPSTREAM   ( 5 * 470000 )
#define VOLTAGE_RESISTOR_DOWNSTREAM ( 1000 )

#define LED_POWER_OFF_VALUE 5
#define BUTTON_DELAY 500

#define MQTT_SERVER "iot.eclipse.org"
#define MQTT_PORT 1883

#endif

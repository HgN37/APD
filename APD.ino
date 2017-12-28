#include "task.h"

/* SETUP FUNCTION
 *  - Thiết lập các GPIO cần thiết.
 *  - Thiết lập cổng giao tiếp nối tiếp để debug.
 *  - Khởi động HLW8012.
 */
void setup() {
  taskSetup();
}

/* LOOP FUNCTION
 *  - Kết nối WiFi và giao tiếp MQTT.
 *  - Đọc giá trị ADC để điều khiển PWM.
 *  - Nhận, giải mã và thực thi lệnh từ server.
 */
void loop() {
  // Kiểm tra kết nối
  wifiConnect();
  mqttConnect();

  // Đọc ADC và nút nhấn điều khiển PWM
  adc = (int)(analogRead(A0)*100/1024);
  analogWrite(PIN_PWM, adc);
  if(buttonReadPressed() == true) {
    digitalWrite(PIN_RELAY, !digitalRead(PIN_RELAY));
  }
  
  //! Đọc HLW
  

  yield();
}


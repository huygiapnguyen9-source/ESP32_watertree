#include <Arduino.h>
// ===== Blynk =====
#define BLYNK_TEMPLATE_ID "TMPL6rI_5St8G"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "kmMILX5RZ3cdnWG9-pXQtle5os9XUzl3"
#include <WiFi.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2); // địa chỉ I2C 0x27, 16x2


char ssid[] = "EMI";
char pass[] = "Hieudaoancut";

// ===== Timer =====
BlynkTimer timer;

// ===== Cấu hình cảm biến và relay =====
const int sensor_pin = 36; // Soil moisture sensor 0/P pin
const int relay = 17;      // chân điều khiển relay
bool relay_state = false;   // lưu trạng thái hiện tại

const int threshold_on  = 10;  // ngưỡng bật bơm
const int threshold_off = 50;  // ngưỡng tắt bơm

int sensor_analog = 0;
int moisture = 0;

// ===== Hàm đọc cảm biến và cập nhật LCD/Blynk =====
void updateDisplayAndRelay() {
    // Đọc cảm biến
    sensor_analog = analogRead(sensor_pin);
    moisture = 100 - ((sensor_analog / 4095.0) * 100);

    // Serial Monitor
    Serial.print("ADC Raw = "); Serial.println(sensor_analog);
    Serial.print("Moisture = "); Serial.print(moisture); Serial.println("%");

    // Cập nhật Blynk
    Blynk.virtualWrite(V2, moisture);

    // Kiểm soát relay
    if (moisture < threshold_on && !relay_state) {
        digitalWrite(relay, LOW);   // bật bơm
        relay_state = true;
        Blynk.virtualWrite(V0, HIGH);
    } else if (moisture >= threshold_off && relay_state) {
        digitalWrite(relay, HIGH);  // tắt bơm
        relay_state = false;
        Blynk.virtualWrite(V0, LOW);
        Blynk.virtualWrite(V1, LOW); // tắt button trên Blynk
    }

    // Cập nhật LCD
    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(moisture);
    lcd.print("%   "); // xóa số cũ

    lcd.setCursor(0, 1);
    lcd.print("Pump: ");
    if (relay_state) lcd.print("ON "); 
    else lcd.print("OFF");
}

// ===== Setup =====
void setup() {
    // LCD
    lcd.init();
    lcd.backlight();

    // Serial
    Serial.begin(9600);

    // Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Relay
    pinMode(relay, OUTPUT);
    digitalWrite(relay, HIGH); // mặc định tắt

    // Hiển thị lần đầu
    updateDisplayAndRelay();

    // Timer: mỗi 1s cập nhật
    timer.setInterval(1000L, updateDisplayAndRelay);
}

// ===== Loop =====
void loop() {
    Blynk.run();
    timer.run();
}

// ===== Blynk Button V1 =====
BLYNK_WRITE(V1) {
    int pinValue = param.asInt();
    if (pinValue == 1) {
        digitalWrite(relay, LOW);
        relay_state = true;
        Blynk.virtualWrite(V0, HIGH);
    } else {
        digitalWrite(relay, HIGH);
        relay_state = false;
        Blynk.virtualWrite(V0, LOW);
    }
    // Cập nhật LCD theo nút bấm
    lcd.setCursor(0, 1);
    lcd.print("Pump: ");
    if (relay_state) lcd.print("ON "); 
    else lcd.print("OFF");
}

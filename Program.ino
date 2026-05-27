#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

Servo palang;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int trigPin = 18;
const int echoPin = 19;
const int ledMerah = 26;
const int ledHijau = 25;
const int servoPin = 13;
const int sdaPin = 21;
const int sclPin = 22;
const int batasJarakCm = 10;
const int sudutTertutup = 180;
const int sudutTerbuka = 90;
const unsigned long jedaTutupMs = 1000;
const unsigned long durasiAnimasiTutupMs = 500;

enum StatusPintu {
  TERTUTUP,
  MEMBUKA,
  TERBUKA,
  MENUTUP
};

void bukaPalang();
void tutupPalang();
float bacaJarakCm();
void setLed(bool hijauMenyala, bool merahMenyala);
void updateLampu();
void tampilLCD(int mode);

int lcdMode = -1;  // 0=open, 1=closing, 2=closed
unsigned long waktuSensorKosong = 0;
unsigned long waktuMenutup = 0;
StatusPintu statusPintu = TERTUTUP;

void tampilLCD(int mode) {
  if (lcdMode == mode) return;

  lcdMode = mode;
  lcd.clear();

  if (mode == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Pintu Terbuka");
  } else if (mode == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Pintu Menutup");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Silahkan");
    lcd.setCursor(0, 1);
    lcd.print("Mendekat");
  }
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledHijau, OUTPUT);
  pinMode(ledMerah, OUTPUT);

  digitalWrite(trigPin, LOW);

  Wire.begin(sdaPin, sclPin);
  lcd.init();
  lcd.backlight();

  ESP32PWM::allocateTimer(0);
  palang.setPeriodHertz(50);
  palang.attach(servoPin, 500, 2400);

  tutupPalang();
  tampilLCD(2);
  updateLampu();
}

void loop() {
  float jarak = bacaJarakCm();
  bool adaObjek = jarak > 0 && jarak <= batasJarakCm;

  if (adaObjek) {
    if (statusPintu == TERTUTUP || statusPintu == MENUTUP) {
      statusPintu = MEMBUKA;
      updateLampu();
      bukaPalang();
      statusPintu = TERBUKA;
    }

    waktuSensorKosong = 0;
    tampilLCD(0);
  } else {
    if (waktuSensorKosong == 0) {
      waktuSensorKosong = millis();
    }

    if (statusPintu == MENUTUP) {
      if (millis() - waktuMenutup >= durasiAnimasiTutupMs) {
        statusPintu = TERTUTUP;
        tampilLCD(2);
      }
    } else if (statusPintu == TERBUKA) {
      if (millis() - waktuSensorKosong >= jedaTutupMs) {
        statusPintu = MENUTUP;
        tampilLCD(1);
        updateLampu();
        tutupPalang();
        waktuMenutup = millis();
        waktuSensorKosong = 0;
      } else {
        tampilLCD(0);
      }
    } else {
      tampilLCD(2);
    }
  }

  updateLampu();
  delay(200);
}

float bacaJarakCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long durasi = pulseIn(echoPin, HIGH, 30000);
  if (durasi == 0) {
    return -1;
  }

  return durasi * 0.0343f / 2.0f;
}

void setLed(bool hijauMenyala, bool merahMenyala) {
  digitalWrite(ledHijau, hijauMenyala ? HIGH : LOW);
  digitalWrite(ledMerah, merahMenyala ? HIGH : LOW);
}

void updateLampu() {
  if (statusPintu == MEMBUKA || statusPintu == TERBUKA) {
    setLed(true, false);
  } else {
    setLed(false, true);
  }
}

void bukaPalang() {
  palang.write(sudutTerbuka);
}

void tutupPalang() {
  palang.write(sudutTertutup);
}

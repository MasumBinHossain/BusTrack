#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define RST_PIN 48
#define SS_PIN 53
#define RED_LED_PIN 3
#define BUZZER_PIN 14

MFRC522 rfid(SS_PIN, RST_PIN);
SoftwareSerial gsm(16, 17);

LiquidCrystal_I2C lcd(0x27, 16, 2);

String authorizedCards[] = {
  ""
};

void setup() {
  Serial.begin(9600);
  gsm.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void loop() {

  rfid.PICC_HaltA();
}

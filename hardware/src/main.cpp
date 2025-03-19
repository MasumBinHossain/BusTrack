#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

//Pins
#define RST_PIN 48
#define SS_PIN 53
#define LED_PIN 3
#define BUZZER_PIN 7
#define GPS_RX 19
#define GPS_TX 18
#define ESP_RX 14
#define ESP_TX 15

// Initialize RFID
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;

// Constants
const String VALID_UID = "23D1FCFD";  // Your UID
const String PHONE_NUMBER = "+8801850416701";
const String STUDENT_NAME = "Masum";
const String STUDENT_ROLL = "201-15-14182";

// Variables
String currentUID = "";
bool isEntered = false;
float latitude = 0.0;
float longitude = 0.0;

float dlatitude = 23.876879629849153;   // Default latitude
float dlongitude = 90.32003161237338;  // Default longitude

void setup() {
  // Serial connections
  Serial.begin(115200);      // Debug
  Serial1.begin(9600);       // GPS (TX1, RX1)
  Serial2.begin(9600);     // GSM on Serial2 (pins 16,17)
  Serial3.begin(115200);     // ESP8266 (TX3, RX3)

  // RFID initialization
  SPI.begin();
  rfid.PCD_Init();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  centerText("BusTrack", 0);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize GSM
  initGSM();

  Serial.println("System Initialized");
}

void loop() {

   updateGPSData();


  static unsigned long lastLocationUpdate = 0;
  if (millis() - lastLocationUpdate > 10000) { // Every 10 seconds
   // sendLocationToWebsite(); //need to change
    String gpsData = ""; 
    if(latitude<3)gpsData += "http://maps.google.com/maps?q=" + String(dlatitude, 6) + "," + String(dlongitude, 6);
    else gpsData += "http://maps.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6);
    Serial3.println("GPS:" + gpsData);
    Serial.println("GPS Sent: " + gpsData);
    lastLocationUpdate = millis();
  }
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    processRFIDCard();
  }
  // Check for RFID
  
  

  // Check for GPS data
  
  
  /*
  if (Serial1.available()) {
    String gpsData = "";
    while (Serial1.available()) {
      char c = Serial1.read();
      gpsData += c;
    }
    // Parse GPS data if needed (here we send raw data for simplicity)
    
  }*/

  delay(500);
}

void centerText(String text, int row) {
  int spaces = (16 - text.length()) / 2;
  lcd.setCursor(spaces, row);
  lcd.print(text);
}

void initGSM() {
  lcd.clear();
  centerText("Init GSM...", 0);
  
  delay(2000);
  
  Serial2.println("AT");
  delay(1000);
  
  Serial2.println("AT+CMGF=1");
  delay(1000);
  
  Serial2.println("AT+CSCS=\"GSM\"");
  delay(1000);
  
  displayTemporaryMessage("GSM Ready", 1000);
}

void updateGPSData() {
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
    }
  }
}
void processRFIDCard() {
  currentUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    currentUID += String(rfid.uid.uidByte[i], HEX);
  }
  currentUID.toUpperCase();
  
  if (currentUID == VALID_UID) {
    handleValidCard();
  } else {
    handleInvalidCard();
  }
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void handleValidCard() {
  isEntered = !isEntered;
  String status = isEntered ? "Entered" : "Exited";
  
  // Update LCD
  lcd.clear();
  centerText(STUDENT_ROLL, 0);
  centerText(status + " The Bus", 1);
  
  // Short beep for valid card
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100); 
  digitalWrite(BUZZER_PIN, LOW);

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  String location_url = "http://maps.google.com/maps?q=";
  if(latitude<3)location_url += String(dlatitude, 6) + "," + String(dlongitude, 6);
  else location_url += String(latitude, 6) + "," + String(longitude, 6);


  String sheet_data= "?UID=" + uid + "&ID=" + STUDENT_ROLL + "&name=" + STUDENT_NAME + "&status=" + status + "&location=" + location_url;
  uid.toUpperCase();
  Serial3.println("RFID:" + sheet_data); // Send UID to ESP8266
  Serial.println("RFID Sent: " + sheet_data);
  
  // Send SMS
  String message = STUDENT_NAME + " " + status + " The Bus\nLocation: " + location_url;
  
  sendSMS(message);
  
  // Send to website
 // sendToWebsite(status); ///need to change
  
  // Return to default message after 2 seconds
  delay(2000);
  lcd.clear();
  centerText("BusTrack", 0);
}

void handleInvalidCard() {
  lcd.clear();
  centerText("Access Denied", 0);
  
  // Long beep and LED for invalid card'
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(2000);  // Long 2-second beep
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Return to default message
  delay(500);
  lcd.clear();
  centerText("BusTrack", 0);
}

void displayTemporaryMessage(String message, int duration) {
  lcd.clear();
  centerText(message, 0);
  delay(duration);
  lcd.clear();
  centerText("BusTrack", 0);
}

void sendSMS(String message) {
  Serial2.println("AT+CMGS=\"" + PHONE_NUMBER + "\"");
  delay(1000);
  Serial2.println(message);
  delay(100);
  Serial2.write(26); // End SMS
  delay(1000);
}

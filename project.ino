#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> ///ดึง LCD มาใช้
//ระบุชื่อ Wi-fi ที่จะต่อ
#define WIFI_SSID "N~Alzheimer~N"
#define WIFI_PASSWORD "21052539."
//กำหนดลิ้ง Firebase ที่จะเชื่อม
#define FIREBASE_HOST "uidofrfid.firebaseio.com"
#define FIREBASE_AUTH "iE3hL21pDJQn5MPw5uC20MATTneWzac97s7879Go"
//กำหนดขา RFID
#define SS_PIN D4
#define RST_PIN D3
MFRC522 mfrc522(SS_PIN, RST_PIN);
#define Buz D0
LiquidCrystal_I2C lcd(0x3F, 16, 2); //สร้างตัวแปร lcd

void setup() { 
  pinMode(Buz,OUTPUT);
  Serial.begin(115200);
  SPI.begin(); //ให้เริ่มทำงาน SPI
  mfrc522.PCD_Init(); //rfid เริ่มทำงาน
  lcd.begin(); // lcd เริ่มทำงาน
  
   // เชื่อม wifi.
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.print("Connecting"); //Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print("."); //Serial.print(".");
    delay(500);
  }
  Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected "); //Serial.print("connected: ");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP()); //Serial.println(WiFi.localIP());
  //เชื่อม Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  digitalWrite(Buz, HIGH), delay(100), digitalWrite(Buz, LOW);

}

void loop() {
  //ส่วนของการอ่าน รหัส RFID
 // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" UID tag :"); //Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();  ///รหัส UID อยู๋ในตัวแปร Content แล้ว
  lcd.setCursor(0,1);
  lcd.print(content);

  digitalWrite(Buz, HIGH), delay(200), digitalWrite(Buz, LOW);
  delay(2000);

  ///ส่งรหัสไป Firebase
  String name = Firebase.pushString("Customer", content);
   if (Firebase.failed()) {
      lcd.clear();
      lcd.print("pushing /Customer failed:");
      //Serial.print("pushing /Customer failed:");
      Serial.println(Firebase.error());  
      return;
  }
  lcd.clear();
  lcd.print("pushed: /Customer/");
  //Serial.print("pushed: /Customer/");
  Serial.println(name);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

}

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> ///ดึง LCD มาใช้
#include <time.h>
//ระบุชื่อ Wi-fi ที่จะต่อ
#define WIFI_SSID "WTF"//"Rm.HW-Club-5G"//"N~Alzheimer~N"//Riffan Internet 2.4G"" 
#define WIFI_PASSWORD "xxxxxxxx"//"5935512007"//"21052539."//"16240828"

//กำหนดลิ้ง Firebase ที่จะเชื่อม
#define FIREBASE_HOST "uidofrfid.firebaseio.com"
#define FIREBASE_AUTH "W5wX0Zia2wOVMRfSO6Z5cJFuwxgGCyU5bv9C4A7U"

int timezone = 7 * 3600; //ตั้งค่า TimeZone ตามเวลาประเทศไทย
int dst = 0; //กำหนดค่า Date Swing Time
String ntp_time = "";
//กำหนดขา RFID
#define SS_PIN D4
#define RST_PIN D3
MFRC522 rfid(SS_PIN, RST_PIN);
#define Buz D0
LiquidCrystal_I2C lcd(0x3F, 16, 2); //สร้างตัวแปร lcd

void WifiConfig();
void NowTime();
void setup() { 
  
  pinMode(Buz,OUTPUT);
  Serial.begin(115200);
  SPI.begin(); //ให้เริ่มทำงาน SPI
  rfid.PCD_Init(); //rfid เริ่มทำงาน
  lcd.begin(); // lcd เริ่มทำงาน
  WifiConfig();
  
  digitalWrite(Buz, HIGH), delay(100), digitalWrite(Buz, LOW);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  NowTime();
 
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();

  Serial.println("key: ");
  lcd.setCursor(2,1);
  lcd.print(strID);
  delay(200);
  lcd.setCursor(0,1);
  lcd.clear();

  digitalWrite(Buz, HIGH), delay(200), digitalWrite(Buz, LOW);
  delay(2000);

  ///ส่งรหัสไป Firebase
 StaticJsonBuffer<200> jsonBuffer;
  JsonObject& valueObject = jsonBuffer.createObject();
  valueObject["uid"] = strID;
  valueObject["status"] = 0;
  String name = Firebase.push("uid", valueObject);
  
   if (Firebase.failed()) {
      lcd.setCursor(2,1);
      lcd.print("Pushing failed");
      delay(500);
      lcd.setCursor(0,1);
      lcd.clear();
      Serial.print("pushing failed");
      Serial.println(Firebase.error());  
      return;
  }
  lcd.setCursor(5,1);
  lcd.print("Pushed");
  delay(500);
  lcd.setCursor(0,1); 
  lcd.clear();
  Serial.print("pushed");
  Serial.println(name);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

}
void NowTime(){
   time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
 
  ntp_time = String(p_tm->tm_hour);
  ntp_time += ":";
  ntp_time += String(p_tm->tm_min);
  ntp_time += ":";
  ntp_time += String(p_tm->tm_sec);
  lcd.setCursor(5, 0); 
  lcd.print(ntp_time);
}
void WifiConfig(){
   // เชื่อม wifi.
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.print("Connecting"); //Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print("."); //Serial.print(".");
    delay(500);
  }
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected "); //Serial.print("connected: ");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP()); //Serial.println(WiFi.localIP());
  delay(2000);
  lcd.clear();
}

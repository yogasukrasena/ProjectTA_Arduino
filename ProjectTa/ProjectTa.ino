#include <SoftwareSerial.h> 
#include <Adafruit_Fingerprint.h>
#include <DS3231.h> //mengincludekan library DS3231
SoftwareSerial fingerprint(2, 3); // pin RX | TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprint);
DS3231  rtc(A4, A5);
#define relay1 8
#define relay2 9
#define relay3 10
#define relay4 11
#define buzzer 5

uint8_t id;

int analogPin = A0; // pin arduino yang terhubung dengan pin S modul sensor tegangan
 
float Vmodul = 0.0; 
float hasil = 0.0;
float R1 = 30000.0; //30k
float R2 = 7500.0; //7500 ohm resistor, 
int value = 0;
  
String data;
int hourSet = 0;
int timeSet = 0;
int timeoff = 01;
int getNowTime;
int statusTime = 0;
int statusTimer = 0;
int statusMode = 0;
int statusMesin = 0;
int enrollStatus = 0;
Time t;  

int test = 1;
           
void setup() 
{
  pinMode(analogPin, INPUT); //Sensor Tegangan
  
  Serial.begin(9600);
  finger.begin(57600);
  rtc.begin();

//  rtc.setDate(12, 2, 2021);   //mensetting tanggal 07 april 2018
//  rtc.setTime(20, 06, 00);     //menset jam 22:00:00
//  rtc.setDOW(5);     //menset hari "Sabtu"

//  if (finger.verifyPassword()) {
//    Serial.println("FingerPrint Sensor Ditemukan!");
//  } else {
//    Serial.println("FingerPrint Sensor Tidak Ditemukan! :(");
//    while (1) {
//      delay(1);
//    }
//  }
// 
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
}

void buzzerTime(bool p){
  if(p){
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500);
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500); 
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
  }else{
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500); 
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500); 
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
  } 
}

void buzzerGuest(bool p){
  if(p){
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500); 
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
  }else{
    tone(buzzer, 3000);
    delay(500);        
    noTone(buzzer);
    delay(500); 
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
    tone(buzzer, 3000);
    delay(250);        
    noTone(buzzer);
  } 
}

void buzzerOk(){
  tone(buzzer, 3000);
  delay(100);        
  noTone(buzzer);
}

void buzzerFail(){
  tone(buzzer, 3000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
}

//-------------------------------------------------------FUNGSI FINGERPRINT-----------------------------------------
void ENROLL() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = 1;
  if (id == 0) {// ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  getFingerprintEnroll();
  enrollStatus = 0;
}

//uint8_t readnumber(void) {
//  uint8_t num = 0;
//
//  while (num == 0) {
//    while (! Serial.available());
//    num = Serial.parseInt();
//  }
//  return num;
//}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  buzzerOk();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

   p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  buzzerOk();
  buzzerOk();      
  Serial.println("Remove finger");
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    buzzerFail();
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    buzzerOk();
    buzzerOk();
    buzzerOk();
    enrollStatus = 0;
    delay(2000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

//-------------------------------------END ENROLL------------------------------
//-------------------------------------CEK FINGERPRINT------------------------------

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    buzzerOk();
    if(statusMesin == 0){
      Serial.println("Found a print match!");
      if(statusMode == 0){
          digitalWrite(relay2,LOW);
          startEngine();
        }else if(statusMode == 1){
          startEngine();
        }
    }
    else if(statusMesin == 1){
      if(statusMode == 0){
          digitalWrite(relay2,HIGH);
          stopEngine();
        }else if(statusMode == 1){
          stopEngine();
        }  
    }
    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    buzzerFail();
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
//-------------------------------------------END CEK FINGERPRINT------------------------------------
//-------------------------------------------DELETE FINGERPRINT------------------------------------
void DELETE() {
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  uint8_t id = 1;
  if (id == 0) {// ID #0 not allowed, try again!
    return;
  }
  Serial.print("Deleting ID #");
  Serial.println(id);
  deleteFingerprint(id);
}

//uint8_t readnumber(void) {
//  uint8_t num = 0;
//
//  while (num == 0) {
//    while (! Serial.available());
//    num = Serial.parseInt();
//  }
//  return num;
//}

//----RETURN NUM-------------------//


uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }
}
//-----------------------------------------END DELETE FINGERPRINT-----------------------
  

void cekTegangan(){

  value = analogRead(analogPin);
  Vmodul = (value * 5.0) / 1024.0;
  hasil = Vmodul / (R2/(R1+R2));
  Serial.print(hasil,2);
  Serial.print("volt");
   
}

void startEngine()
{
  Serial.print("01"); //Ignition On
  digitalWrite(relay1,LOW);
  delay(5000);
  Serial.print("02"); //Starting Engine
  digitalWrite(relay3,LOW);
  digitalWrite(relay4,LOW);
  delay(500);
  Serial.print("03"); //Engine Running
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
  if(statusTimer == 0){
    statusMesin = 1;
  }
  
  
}

void stopEngine(){

  digitalWrite(relay1,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
  statusMesin = 0;
}

void guestModeOn(){
  Serial.print("04"); //Guest Mode on
  digitalWrite(relay2,LOW);
  if(statusTimer == 1){
    statusTimer = 0;
  }
}

void guestModeOff(){
  Serial.print("05"); //Guest Mode off
  if(statusMesin == 0){
    digitalWrite(relay2,HIGH);
  }
}


void backFire(){
  digitalWrite(relay1,HIGH);
  delay(500);
  digitalWrite(relay1,LOW);
}

void setTimer(){
  t = rtc.getTime();
//  Serial.print(timeSet);
//  Serial.println(t.min);
//  Serial.print(hourSet);
//  Serial.println(t.hour);

  if(statusMesin == 0){
    if((timeSet == t.min)&&(hourSet == t.hour)){
      if(statusTime == 0){
        if(statusMode == 0){
            digitalWrite(relay2,LOW);
            startEngine();
          }else if(statusMode == 1){
            startEngine();
          }
        getNowTime = t.min + timeoff;
        Serial.print(getNowTime);
        statusTime = 1;
      }
      if(statusTime == 1){
//        Serial.print("Sampai di status");
//        Serial.print(getNowTime);
//        Serial.print(t.min);
//        Serial.print("\n");
      }
    }
    if(t.min == getNowTime){
      Serial.print("Sampai di status stop");
      if(statusMode == 0){
        digitalWrite(relay2,HIGH);
        stopEngine();
      }else if(statusMode == 1){
        stopEngine();
      }  
      statusTime = 0;
      getNowTime = 0;
    }
  }
}

void fungsiUtama(){
  if(Serial.available() > 0){
      
      data = Serial.readString();
//      Serial.println(data);
      
      if(data == "0"){
        buzzerOk();
        if(statusMode == 0){
          digitalWrite(relay2,LOW);
          startEngine();
        }else if(statusMode == 1){
          startEngine();
        }
      }
      else if(data == "1"){
         Serial.print("00"); //Engine Stop
         buzzerOk();
         buzzerOk();
        if(statusMode == 0){
          digitalWrite(relay2,HIGH);
          stopEngine();
        }else if(statusMode == 1){
          stopEngine();
        }       
      }
      else if(data == "2"){
        buzzerTime(true);
        Serial.print("06"); //Timer On
        statusTimer = 1;
      }
      else if(data == "3"){
        buzzerTime(false);
        Serial.print("07"); //Timer Off
        if(statusMode == 0){
          digitalWrite(relay2,HIGH);
          stopEngine();
        }else if(statusMode == 1){
          stopEngine();
        }  
        statusTimer = 0;
      }
      else if(data == "5"){
        buzzerGuest(true);
        statusMode = 1;
        guestModeOn();
      }
      else if(data == "4"){
        buzzerGuest(false);
        statusMode = 0;
        guestModeOff();
      }
      else if(data == "6"){
        cekTegangan(); //Cek Tegangan Aki Motor
      }
      else if(data == "7"){
        backFire();
      }
      else if(data == "8"){
          enrollStatus = 1; // Daftar Fingerprint
      }
      else if(data == "9"){
          DELETE(); // Daftar Fingerprint
      }
      else if(data.startsWith("hrs")){
        data.replace("hrs","");
//        Serial.println(data);
        char text[10];
        data.toCharArray(text,10);
//        Serial.println(text);
        int n = sscanf(text, "%d:%d", &hourSet, &timeSet);
        Serial.print(hourSet);
        Serial.print(':');
        Serial.print(timeSet);
      }
      
    }
}
void loop(){
//  if(enrollStatus == 1){
//    ENROLL();
//  }
//  else if(enrollStatus == 0){
//    getFingerprintID();
//  }
  
  //Jika Timer Di nonaktifkan
  if(statusTimer == 0){
    fungsiUtama();
  }

  //Jika Timer Dihidupkan
  if(statusTimer == 1){
    setTimer();
    fungsiUtama();
  }
  
                              
 
}

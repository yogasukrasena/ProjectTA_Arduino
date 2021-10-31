#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>

//SoftwareSerial fpSerial(2,3);//RX,TX
//SoftwareSerial espSerial(4,5);
//SoftwareSerial pulseSerial(6,7);

#define espSerial Serial1
#define pulseSerial Serial2
#define gpsSerial Serial3
SoftwareSerial fpSerial(10,11);

//ultrasonic define pin 
#define echoFront A1 //echo pin
#define trigFront A0 //trig pin
#define echoRight A3
#define trigRight A2 
#define echoLeft A5 
#define trigLeft A4 

#define soilPin 10 
#define buttonPin 2
#define buzzer 8

//vibration pin
#define vibFront 11
#define vibRight 12
#define vibLeft 13

// variables will change:
int buttonState = 0, cekfinger = 0;
int soilValue, buttonValue;
// variabel gps;
float latVar, lngVar;
int gpsStatus;
// variabel untuk fingerprint
boolean sidikJari = false;
int id, fingerID;
String dataID, dataPulse;
String separator = "|";

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);
TinyGPSPlus gps;

void setup() {  
  Serial.begin(9600);    
  finger.begin(57600);
  // cek id fingerprint  
  for (int finger = 1; finger < 6; finger++) {    
    if(cekfinger == 1){
      finger = finger-1;      
    }
    downloadFingerprintTemplate(finger);      
  }
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    id = 1;
    enrollFinger();    
  }else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");    
  }
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");           
    while(!sidikJari){  
      fingerID = getFingerprintID();
    }   
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
      
  espSerial.begin(9600);  
  pulseSerial.begin(9600);
  gpsSerial.begin(9600);  

//  Serial.print("Trying connect to AP ... ");
//  espSerial.println("espClientMode,mantap_mantap,shittman");
//  while(!espSerial.available()){
//    Serial.print(".");    
//    delay(1000);
//  }
//  Serial.println("Connected !");

   // initialize the pushbutton pin and buzzer  
  pinMode(buzzer, OUTPUT);  
  pinMode(buttonPin, INPUT);

  // intialize the vibrate
  pinMode(vibFront, OUTPUT);
  pinMode(vibRight, OUTPUT);
  pinMode(vibLeft, OUTPUT);

  // initialize the ultrasonic
  pinMode(trigFront, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoFront, INPUT); // Sets the echoPin as an INPUT  
  pinMode(trigRight, OUTPUT); 
  pinMode(echoRight, INPUT); 
  pinMode(trigLeft, OUTPUT); 
  pinMode(echoLeft, INPUT); 
}
//-----------REBOOT SYSTEM------------
void(* rebootSystem) (void) = 0;

//----------------------Cek ID Finger---------------------------------
uint8_t downloadFingerprintTemplate(uint16_t id)
{
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
      dataID += id+separator;
      cekfinger = 0;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      cekfinger = 1;
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      cekfinger = 0;
      return p;
  }

  // OK success!

  Serial.print("Attempting to get #"); Serial.println(id);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" transferring:");
      break;
   default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }

  // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
  uint8_t bytesReceived[534]; // 2 data packets
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
      if (fpSerial.available()) {
          bytesReceived[i++] = fpSerial.read();
      }
  }

  uint8_t fingerTemplate[512]; // the real template
  memset(fingerTemplate, 0xff, 512);

  // filtering only the data packets
  int uindx = 9, index = 0;
  while (index < 534) {
      while (index < uindx) ++index;
      uindx += 256;
      while (index < uindx) {
          fingerTemplate[index++] = bytesReceived[index];
      }
      uindx += 2;
      while (index < uindx) ++index;
      uindx = index + 9;
  }
}
//----------------------End Cek ID Finger---------------------------------
//-------------fungsi ENROLL atau mendaftarkan finggerprint pengguna----------------
void enrollFinger(){
  finger.begin(57600);  
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as..."); 
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  if (id == 334){
     sidikJari = false;
     fingerID = 0;
     rebootSystem();      
  }   
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  while(! getFingerprintEnroll() );   
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
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

  // OK success!

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

  Serial.println("Remove finger");
  delay(2000);
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
    delay(1000);
    rebootSystem();
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
//----------------End ENROLL atau mendaftarkan finggerprint--------------
//----------------Fungsi Scan finggerprint-----------------------

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
//      wifiSerial.print("S");
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
    Serial.println("Found a print match!");    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);  
  sidikJari = true;
//  tone(buzzer, 1000);
//  delay(1000);
//  noTone(buzzer);  
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
//--------------------End Scan finggerprint-----------------------
//------------------Fungsi Delete finggerprint--------------------

void deleteFinger(){
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");  
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  if (id == 334){
     rebootSystem(); 
  }  
  Serial.print("Deleting ID #");
  Serial.println(id);
  
  deleteFingerprint(id);    
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {    
    Serial.println("Deleted!");
    delay(1000);    
    rebootSystem();
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
//-------------------End Delete finggerprint----------------------
//-------------------Fungsi Button Push---------------------------
int Button() {
  buttonState = digitalRead(buttonPin);
  int i = buttonState;   
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    buttonValue = constrain(buttonValue,0,4);
    buttonValue += i;                                           
  } else {
    // turn LED off:
    buttonValue = 0;       
  }
}
//-------------------End push button---------------------------
//-------------------Fungsi Sensor Ultrasonic------------------
int UltraFront(){
  
  long durationFront; 
  int distanceFront;  

  // Clears the trigPin condition
  digitalWrite(trigFront, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigFront, HIGH); delayMicroseconds(10);
  digitalWrite(trigFront, LOW);    
  durationFront = pulseIn(echoFront, HIGH);
  // Calculating the distance
  distanceFront = durationFront * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)  
  return distanceFront;
}

int UltraRight(){
  
  long durationRight; 
  int distanceRight;  

  // Clears the trigPin condition
  digitalWrite(trigRight, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigRight, HIGH); delayMicroseconds(10);
  digitalWrite(trigRight, LOW);    
  durationRight = pulseIn(echoRight, HIGH);
  // Calculating the distance
  distanceRight = durationRight * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distanceRight;
}

int UltraLeft(){
  
  long durationLeft; 
  int distanceLeft;  

  // Clears the trigPin condition
  digitalWrite(trigLeft, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigLeft, HIGH); delayMicroseconds(10);
  digitalWrite(trigLeft, LOW);    
  durationLeft = pulseIn(echoLeft, HIGH);
  // Calculating the distance
  distanceLeft = durationLeft * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distanceLeft;
}
//-------------------End Sensor Ultraasonic------------------------
//----------------------Fungsi Utama----------------------------
void mainFunction(){         
    Serial.print("Fingger ID :");
    Serial.println(fingerID);
    //print ultrasonic
    Serial.print("Distance center: ");
    Serial.print(UltraFront());
    Serial.println(" cm"); 
    Serial.print("Distance right: ");
    Serial.print(UltraRight());
    Serial.println(" cm"); 
    Serial.print("Distance left: ");
    Serial.print(UltraLeft());
    Serial.println(" cm");
    //print soil
    Serial.print("nilai soil :");
    Serial.println(soilValue = analogRead(soilPin));                 
    //kondisi vibrate on
    if(UltraFront()<100){
      digitalWrite(vibFront, HIGH); //vibrate          
    }
    else{
      digitalWrite(vibFront, LOW); //stop vibrate
    }
    if(UltraRight()<100){
      digitalWrite(vibRight, HIGH); //vibrate          
    }else{
      digitalWrite(vibRight, LOW); //stop vibrate          
    }
    if(UltraLeft()<100){
      digitalWrite(vibLeft, HIGH); //vibrate          
    }else{
      digitalWrite(vibLeft, LOW); //stop vibrate          
    }                 
}
//-------------------End Fungsi Utama----------------------

void loop() {      
  mainFunction();
  gpsData();    
  Button();
  Serial.print("Nilai Button : ");Serial.println(buttonValue);
  Serial.print("Data Finger : ");Serial.println(dataID);
  Serial.println("---------------------------------");
  Serial.print("espFirebase,"+String(gpsStatus)+","+dataID+","+String(buttonValue)+","+"100,");
  Serial.print(String(latVar,6)+","+String(lngVar,6)+",");
  Serial.println(dataPulse);
  // kirim data ke esp8266
  espSerial.print("espFirebase,"+String(gpsStatus)+","+dataID+","+String(buttonValue)+","+"100,");
  espSerial.print(String(latVar,6)+","+String(lngVar,6)+",");
  espSerial.println(dataPulse);  
  while(pulseSerial.available()){
    pulseSerial.setTimeout(100);
    dataPulse = pulseSerial.readString();      
  }  
//  while(espSerial.available()){
//   espSerial.setTimeout(100);       
//   String d = espSerial.readString();
//   if(d.startsWith("enfinger")){
//      d.replace("enfinger","");
//      id = d.toInt();
//      Serial.println(d);
//    }
//  }
}

//----------------------Fungsi upload data GPS--------------------------  
void gpsData(){
  while(gpsSerial.available()>0){
    if (gps.encode(gpsSerial.read())) {
      Serial.println("---------------Data GPS----------------");
      tampilkan();   
    }
  }
  if(millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println("No GPS detected: check wiring.");
  }
}
//----------------------End Fungsi upload data GPS------------------------ 
//----------------------Fungsi Show data GPS-----------------------------
void tampilkan() {
  Serial.print("Fix : ");Serial.println(gps.location.age());
  if(gps.location.age()>1500){
    Serial.print("Location = ");
    Serial.print(latVar,6);
    Serial.print(",");
    Serial.println(lngVar,6);
    Serial.println("Data GPS lama");
    gpsStatus = 0;
  }else if(gps.location.isValid()){
    latVar = gps.location.lat();
    lngVar = gps.location.lng();
    
    Serial.print("Location = ");
    Serial.print(latVar,6);
    Serial.print(",");
    Serial.println(lngVar,6);
    gpsStatus = 1;    
  }else{
    Serial.println("Lokasi Belum Terbaca");    
    latVar = 0; lngVar = 0;
    gpsStatus = 0;
  }
}
//------------------------End Fungsi Show data GPS------------------------

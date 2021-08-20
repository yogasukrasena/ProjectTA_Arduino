#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//serial tx(16) dan rx (17) untuk nodemcu
#define wifiSerial Serial2

// serial tx(14) dan rx(15) untuk finggerprint
#define fpSerial Serial3

//ultrasonic define pin
#define echoFront A2 //echo pin
#define trigFront A1 //trig pin
#define echoRight A4
#define trigRight A3 
#define echoLeft A6 
#define trigLeft A5 

const int soilPin = A0; 
const int buttonPin = 2;     // the number of the pushbutton pin
const int buzzer = 9;        // buzzer to arduino pin 9
const int ledPin =  13;      // the number of the LED pin

//vibration pin
const int vibFront = 3;
const int vibRight = 4;
const int vibLeft = 5;

//variabel timer on alat
unsigned long milisec;
unsigned long detik;
unsigned long menit;
unsigned long jam;

boolean sidikJari = false;

// id untuk fingerprint
int id;
String dataID;
String separator = ",";

// variables will change:
int buttonState = 0;
int soilValue;
int fingerID;
int fingerValid;
int buttonValue;
int kondisi;
int dataInt;
int statusDevice;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);

void setup()
{
  Serial.begin(9600);
  wifiSerial.begin(9600);  
    
  finger.begin(57600);  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  // cek id fingerprint
  for (int finger = 1; finger < 10; finger++) {
    downloadFingerprintTemplate(finger);
  }

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    id = 1;
    enrollFinger();    
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");    
  }

  // initialize the pushbutton pin and buzzer
  pinMode(ledPin, OUTPUT);
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

//--------read input serial---------
int readnumber(void) {
  int num = 0;

  while (num == 0) {
    while (! wifiSerial.available());
    num = dataInt;
  }
  return num;
}
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
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
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
//  Serial.print(i); Serial.println(" bytes read.");
//  Serial.println("Decoding packet...");

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

void printHex(int num, int precision) {
    char tmp[16];
    char format[128];

    sprintf(format, "%%.%dX", precision);

    sprintf(tmp, format, num);
    Serial.print(tmp);
}
//----------------------End Cek ID Finger---------------------------------
//-------------fungsi ENROLL atau mendaftarkan finggerprint pengguna----------------
void enrollFinger(){  
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
      wifiSerial.print("S");
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
  tone(buzzer, 1000);
  delay(1000);
  noTone(buzzer);  
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
    Serial.print("Push Button : ");
    Serial.println(buttonValue);
                         
    return buttonValue;
  } else {
    // turn LED off:
    buttonValue = 0;
    Serial.print("unpush button : ");
    Serial.println(buttonValue);
        
    return buttonValue;     
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
//--------------Sensor Soil atau kelembaban surface----------------
int soilNilai(){

  int soilValue;  
  int limit = 300; 

  soilValue = analogRead(soilPin); 
  Serial.println("Analog Value : ");
  Serial.println(soilValue);

  return soilValue;
}
//-------------------End Sensor Soil---------------------------
//-------------------Fungsi Lifetime alat----------------------
void timerAlat(){
  milisec = millis();
  detik = milisec/1000;
  menit = detik/60;
  jam   = menit/60;

  milisec %= 1000;
  detik %= 60;
  menit %= 60;
  jam %= 24;  
}
//----------------------End Lifetime----------------------------
//----------------------Fungsi Utama----------------------------
void mainFunction(){         
    Serial.print("Fingger ID :");
    Serial.println(fingerID);
    //print ultrasonic
    Serial.print("Distance center: ");
    Serial.print(UltraFront());
    Serial.println(" cm"); 
//    Serial.print("Distance right: ");
//    Serial.print(UltraRight());
//    Serial.println(" cm"); 
//    Serial.print("Distance left: ");
//    Serial.print(UltraLeft());
//    Serial.println(" cm");
    //print soil
    Serial.print("nilai soil :");
    Serial.println(soilValue = analogRead(soilPin));                 
    //kondisi vibrate on
    if(UltraFront()<100){
      digitalWrite(vibFront, HIGH); //vibrate          
    }
//    else{
//      digitalWrite(vibFront, LOW); //stop vibrate
//    }
//    if(UltraRight()<100){
//      digitalWrite(vibRight, HIGH); //vibrate          
//    }else{
//      digitalWrite(vibRight, LOW); //stop vibrate          
//    }
//    if(UltraLeft()<100){
//      digitalWrite(vibLeft, HIGH); //vibrate          
//    }else{
//      digitalWrite(vibLeft, LOW); //stop vibrate          
//    }                 
}
//-------------------End Fungsi Utama----------------------

void loop() {  
  wifiSerial.setTimeout(100);
  fingerID = getFingerprintID();    
  while(sidikJari){    
    timerAlat();    
    wifiSerial.print(0xAA); wifiSerial.print(":");
    wifiSerial.print(100); wifiSerial.print(":");    
    wifiSerial.print(Button()); wifiSerial.print(":");
    wifiSerial.print(dataID); wifiSerial.print(":");
    wifiSerial.print(jam); wifiSerial.print(":");
    wifiSerial.println(menit);
    Serial.println(dataID);
    Serial.println("---------------------------------");
    mainFunction();
    delay(100);    
    while(wifiSerial.available()>0) {
      String data = wifiSerial.readString();
      if(data.startsWith("enfinger")){
        data.replace("enfinger","");
        id = data.toInt();
        enrollFinger();           
      }else if(data.startsWith("delfinger")){
        data.replace("delfinger","");
        id = data.toInt();
        deleteFinger();           
      }   
    }
  }
}

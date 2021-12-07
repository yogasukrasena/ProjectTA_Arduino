#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>
#include <DFRobotDFPlayerMini.h>
#include <NewPing.h>

//SoftwareSerial fpSerial(2,3);//RX,TX
//SoftwareSerial espSerial(4,5);
//SoftwareSerial pulseSerial(6,7);

#define espSerial Serial1
#define pulseSerial Serial2
#define gpsSerial Serial3
SoftwareSerial fpSerial(10,11);
SoftwareSerial voiceSerial(12,13);

//ultrasonic define pin 
#define echoFront A6 //echo pin
#define trigFront A5 //trig pin
#define echoRight A4
#define trigRight A3 
#define echoLeft A1 
#define trigLeft A2
#define MAX_DISTANCE 400

#define soilPin A7 
#define voltCek A0
#define buttonPin 2
#define buzzer 3

//vibration pin
#define vibFront 4
#define vibRight 5
#define vibLeft 6
#define vibSoil 7

// variables will change:
int buttonState = 0;
int soilValue, buttonValue;

// variabel gps;
float latVar, lngVar;
int gpsStatus;

// variabel untuk fingerprint
boolean sidikJari = false;
int failCek = 0;
int id, fingerID, statusFinger;
String dataID, dataPulse;
String separator = ":";

//variabel interval waktu
long previousMillis = 0;
long interval = 1000;
unsigned long currentMillis;

//variable presentase battery
float VOut = 0.0, VIn = 0.0;
float R1 = 30000.0, R2 = 7500.0;
float Vmin = 3.15, Vmax = 4.25;
int battery = 0;
float hasil = 0;

//deklarasi objek library
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial); //finger print
DFRobotDFPlayerMini myDFPlayer; //voice player (modul dfplayer)
TinyGPSPlus gps; // modul GPS neo6m

//ultrasonik define fungsi
NewPing sonarLeft(trigLeft, echoLeft, MAX_DISTANCE);
NewPing sonarRight(trigRight, echoRight, MAX_DISTANCE);
NewPing sonarCenter(trigFront, echoFront, MAX_DISTANCE);

void setup() {  
  Serial.begin(9600);      
  voiceSerial.begin (9600);
  //kode untuk setup voice atau DFplayer mini
  if (!myDFPlayer.begin(voiceSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));  
  myDFPlayer.setTimeOut(100); //Set serial communictaion time out 500ms  
  myDFPlayer.enableDAC();  //Enable On-chip DAC  
  myDFPlayer.volume(30);  //Set volume value (0~30).
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  myDFPlayer.play(1); delay(1000);//memutar sound

  finger.begin(57600);
  // cek id fingerprint  
  cekFinger();
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
    myDFPlayer.play(2);  delay(1000);//memutar sound         
  }
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");           
    while(!sidikJari){
      espSerial.print("rollFinger,");
      espSerial.println("noneAction");  
      fingerID = getFingerprintID();
    }   
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
        
  espSerial.begin(115200);  
  pulseSerial.begin(9600);
  gpsSerial.begin(9600);  
//  analogReference(INTERNAL1V1);  

   // initialize the pushbutton pin and buzzer  
  pinMode(buzzer, OUTPUT);  
  pinMode(buttonPin, INPUT);

  // intialize the vibrate
  pinMode(vibFront, OUTPUT);
  pinMode(vibRight, OUTPUT);
  pinMode(vibLeft, OUTPUT);
  pinMode(vibSoil, OUTPUT);
  
}
//-----------REBOOT SYSTEM------------
void(* rebootSystem) (void) = 0;

//----------------------Cek ID Finger---------------------------------
void cekFinger(){
  for (int finger = 1; finger < 6; finger++) {    
    if(failCek == 1){
      finger = finger-1;      
    }
    downloadFingerprintTemplate(finger);      
  }
}

uint8_t downloadFingerprintTemplate(uint16_t id)
{
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
      dataID += id+separator;
      failCek = 0;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      failCek = 1;
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      failCek = 0;
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
  delay(1000);
  finger.begin(57600);  
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as..."); 
  myDFPlayer.play(6); delay(1000);//memutar sound
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }    
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  while(! getFingerprintEnroll());
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
      espSerial.print("rollFinger,");
      espSerial.println("firstPlace");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");      
      statusFinger = 3;
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      statusFinger = 3;
      break;
    default:
      Serial.println("Unknown error");
      statusFinger = 3;
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
      statusFinger = 3;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      return p;
    default:
      Serial.println("Unknown error");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
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
      espSerial.print("rollFinger,");
      espSerial.println("placeAgain");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      statusFinger = 3;
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      statusFinger = 3;
      break;
    default:
      Serial.println("Unknown error");
      statusFinger = 3;
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
      statusFinger = 3;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      statusFinger = 3;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      return p;
    default:
      Serial.println("Unknown error");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    statusFinger = 3;
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    return p;
  } else {
    Serial.println("Unknown error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) { 
    dataID = "";   
    Serial.println("Stored!");
    statusFinger = 1;
    cekFinger();    
    myDFPlayer.play(7);  delay(1000);//memutar sound    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound    
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound    
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    myDFPlayer.play(8);  delay(1000);//memutar sound    
    statusFinger = 3;
    return p;
  } else {
    Serial.println("Unknown error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound    
    return p;
  }
  return true;
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
      espSerial.setTimeout(100);
      espSerial.print("rollFinger,");
      espSerial.println("noneAction");      
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
      myDFPlayer.play(5);  delay(1000);//memutar sound    
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      myDFPlayer.play(5);  delay(1000);//memutar sound    
      return p;
    default:
      Serial.println("Unknown error");
      myDFPlayer.play(5);  delay(1000);//memutar sound    
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    myDFPlayer.play(5);  delay(1000);//memutar sound    
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    myDFPlayer.play(3);  delay(1000);//memutar sound    
    return p;
  } else {
    Serial.println("Unknown error");
    myDFPlayer.play(5);  delay(1000);//memutar sound    
    return p;    
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);  
  sidikJari = true;
  statusFinger = 0;
  myDFPlayer.play(4);  delay(1000);//memutar sound pertama          
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
  delay(1000);
  finger.begin(57600);
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");  
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }  
  Serial.print("Deleting ID #");
  Serial.println(id);
  myDFPlayer.play(9);  delay(1000);//memutar sound    
  
  deleteFingerprint(id);    
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    dataID = "";
    Serial.println("Deleted!");
    statusFinger = 2;
    myDFPlayer.play(10);  delay(1000);//memutar sound    
    cekFinger();        
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    statusFinger = 3;
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    statusFinger = 3;
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    statusFinger = 3;
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    statusFinger = 3;
    myDFPlayer.play(11);  delay(1000);//memutar sound    
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
int UltraRight(){
  
  int distance, jarak;  
  distance = sonarRight.ping_cm(); //pemanggilan fungsi library
   
  float hasil = (distance + 0.6805) / 0.9967; //kalibrasi hasil pengukuran 
  jarak = round(hasil); //hasil akhir dibulatkan
  
  return jarak;
}

int UltraLeft(){
  
  int distance, jarak;  
  distance = sonarLeft.ping_cm(); //pemanggilan fungsi library
   
  float hasil = (distance + 0.6805) / 0.9967; //kalibrasi hasil pengukuran 
  jarak = round(hasil); //hasil akhir dibulatkan
  
  return jarak;
}

int UltraFront(){
  
  int distance, jarak;  
  distance = sonarCenter.ping_cm(); //pemanggilan fungsi library
   
  float hasil = (distance + 0.6805) / 0.9967; //kalibrasi hasil pengukuran
  jarak = round(hasil); //hasil akhir dibulatkan
  
  return jarak;
}
//-------------------End Sensor Ultraasonic------------------------

//int batteryPercent(){
//  analogReference(INTERNAL1V1);
//  int volt = analogRead(voltCek);// read the input 
//  VOut = (volt * 1.1) / 1024.0;
//  VIn = VOut / (R2/(R1+R2));
////  Serial.print("Input = ");
////  Serial.println(VIn);
//  hasil = ((VIn - Vmin) / (Vmax - Vmin)) * 100;
//  battery = round(hasil);
//  
////  Serial.print("sisa baterai : ");
//  if(battery < 100 && battery > 0){ 
////    Serial.println(battery);  
//    return battery;
//  }else if (VIn < Vmin){
////    Serial.println("0"); 
//    return 0;
//  }else{
////    Serial.println("100");
//    return 100;
//  }
//}

void batteryPercent1(){
//  analogReference(INTERNAL1V1);
  analogReference(INTERNAL2V56);
  int volt = analogRead(voltCek);// read the input 
  VOut = (volt * 2.56) / 1024.0;
  VIn = VOut / (R2/(R1+R2));
  Serial.print("Input = ");
  Serial.println(VIn);
  hasil = ((VIn - Vmin) / (Vmax - Vmin)) * 100;
  battery = round(hasil);
  
//  Serial.print("sisa baterai : ");
  if(battery < 100 && battery > 0){ 
    Serial.println(battery);  
//    return battery;
  }else if (VIn < Vmin){
    Serial.println("0"); 
//    return 0;
  }else{
    Serial.println("100");
//    return 100;
  }
}

//----------------------Fungsi Utama----------------------------
void mainFunction(){   
//  UltraFront();
//  UltraRight();      
//  UltraLeft();
//  soilValue = analogRead(soilPin);
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
//  if(UltraFront()>100){
//    digitalWrite(vibFront, LOW); //vibrate          
//  }else{
//    digitalWrite(vibFront, HIGH); //stop vibrate
//  }
//  if(UltraLeft()>100){
//    digitalWrite(vibLeft, LOW); //vibrate          
//  }else{
//    digitalWrite(vibLeft, HIGH); //stop vibrate          
//  }
//  if(UltraRight()>100){
//    digitalWrite(vibRight, LOW); //vibrate          
//  }else{
//    digitalWrite(vibRight, HIGH); //stop vibrate          
//  }
//    if(soilValue < 500){
//      digitalWrite(vibSoil, HIGH); //vibrate         
//    }else{
//      digitalWrite(vibSoil, LOW); //stop vibrate         
//    }
}
//-------------------End Fungsi Utama----------------------

void loop() {
//  long t1 = millis();        
//  mainFunction();
//  long t2 = millis();
  gpsData();    
//  Button();      
//  Serial.print("Nilai Button : ");Serial.println(buttonValue);
//  Serial.print("Data Finger : ");Serial.println(dataID);
//  Serial.println("---------------------------------");
//  Serial.print("espFirebase,"+String(gpsStatus)+","+dataID+","+String(buttonValue)+","+"100,");
//  Serial.print(String(latVar,6)+","+String(lngVar,6)+",");
//  Serial.println(dataPulse);
  // kirim data ke esp8266
//  currentMillis = millis();
//  if(currentMillis - previousMillis > interval){
//    previousMillis = currentMillis;              
//    espSerial.print("espFirebase,"+String(gpsStatus)+","+String(statusFinger)+","+dataID+","+String(buttonValue)+","+String(batteryPercent())+",");
//    espSerial.print(String(latVar,6)+","+String(lngVar,6)+",");
//    espSerial.println(dataPulse);   
//    Serial.println("data terkirim ke esp");
//    Serial.println(dataID);
//    Serial.println(batteryPercent());    
//  }
//  batteryPercent1();
//  Serial.print("selang waktu : ");Serial.println(t2-t1);
  
  while(pulseSerial.available()){
    pulseSerial.setTimeout(100);
    dataPulse = pulseSerial.readString();          
  } 
  
  while(espSerial.available()){
    espSerial.setTimeout(100);
    String d = espSerial.readString();
    Serial.println(d);
    if(d.startsWith("enfinger") && statusFinger != 1){
      d.replace("enfinger","");
      id = d.toInt();      
      enrollFinger();
    }else if(d.startsWith("delfinger") && statusFinger != 2){
      d.replace("delfinger","");
      id = d.toInt();
      deleteFinger();      
    }else if(d.startsWith("cancel")){
      d.replace("cancel","");
      id = d.toInt();
    }else if(d.startsWith("done")){
      d.replace("done","");
      statusFinger = d.toInt();
    }
  }
//  delay(1000);
}

//----------------------Fungsi upload data GPS--------------------------  
void gpsData(){
  while(gpsSerial.available()>0){
    if (gps.encode(gpsSerial.read())) {
//      Serial.println("---------------Data GPS----------------");
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
//  Serial.print("Fix : ");Serial.println(gps.location.age());
  if(gps.location.age()>2000){
    Serial.print("Location = ");
    Serial.print(latVar,7);
    Serial.print(",");
    Serial.println(lngVar,7);
    Serial.println("Data GPS lama");
    gpsStatus = 0;
  }else if(gps.location.isValid()){
    latVar = gps.location.lat();
    lngVar = gps.location.lng();
    gpsStatus = 1;    
    Serial.print("Location = ");
    Serial.print(latVar,7);
    Serial.print(",");
    Serial.println(lngVar,7);    
  }else{
//    Serial.println("Lokasi Belum Terbaca");    
    latVar = 0; lngVar = 0;
    gpsStatus = 0;
  }
}
//------------------------End Fungsi Show data GPS------------------------

#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>
#include <DFRobotDFPlayerMini.h>
#include <NewPing.h>

//komunikasi serial modul dan esp
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
#define MAX_DISTANCE 300

#define soilPin A7 
#define voltCek A0
#define buttonPin 2

//vibration pin
#define vibFront 4
#define vibRight 5
#define vibLeft 6
#define vibSoil 7

// variables will change:
int buttonState = 0;
int soilValue, buttonValue, connStatus = 1;

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

//variabel interval waktu untuk kondisi
long previousMillis1 = 0;
long interval1 = 10000;
unsigned long currentMillis1;

//variabel interval waktu untuk soil
long previousMillis2 = 0;
long interval2 = 2000;
unsigned long currentMillis2;
int holdSoil = 0;

//variable presentase battery
float VOut = 0.0, VIn = 0.0;
float R1 = 30000.0, R2 = 7500.0;
float Vmin = 3.15, Vmax = 4.20;
int battery = 0, persentase = 0;
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

  myDFPlayer.play(1); delay(3000);//memutar sound

  finger.begin(57600);  
  finger.getTemplateCount();  
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    fingerID = 0; dataID = "0";
    myDFPlayer.play(18); delay(5000);//memutar sound   
    finger.LEDoff();
  }else {
    // cek id fingerprint  
    cekFinger();
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");
    myDFPlayer.play(2);  delay(1000);//memutar sound    
    if (finger.verifyPassword()) {
      finger.LEDon();      
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
  }
  
  espSerial.begin(115200);  
  pulseSerial.begin(9600);
  gpsSerial.begin(9600);  

   // initialize the pushbutton pin
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
  finger.LEDon();  
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
      finger.LEDoff();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      finger.LEDoff();
      return p;
    default:
      Serial.println("Unknown error");
      statusFinger = 3;
      myDFPlayer.play(8);  delay(1000);//memutar sound
      finger.LEDoff();
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
    finger.LEDoff();
    return p;
  } else {
    Serial.println("Unknown error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    finger.LEDoff();
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
    finger.LEDoff();    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    finger.LEDoff();    
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    finger.LEDoff();    
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    myDFPlayer.play(8);  delay(1000);//memutar sound
    finger.LEDoff();    
    statusFinger = 3;
    return p;
  } else {
    Serial.println("Unknown error");
    statusFinger = 3;
    myDFPlayer.play(8);  delay(1000);//memutar sound
    finger.LEDoff();    
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
  myDFPlayer.play(4);  delay(4000);//memutar sound pertama          
  finger.LEDoff();
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
  finger.LEDon();
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");  
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }  
  Serial.print("Deleting ID #");
  Serial.println(id);
//  myDFPlayer.play(9);  delay(5000);//memutar sound    
  
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
    finger.LEDoff();
    cekFinger();        
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    statusFinger = 3;    
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    finger.LEDoff();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    statusFinger = 3;    
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    finger.LEDoff();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    statusFinger = 3;    
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    finger.LEDoff();
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    statusFinger = 3;    
    myDFPlayer.play(11);  delay(1000);//memutar sound    
    finger.LEDoff();
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
  int distance;
  distance = sonarRight.ping_cm(); //pemanggilan fungsi library      
    
  return distance;    
}

int UltraLeft(){
  int distance;
  distance = sonarLeft.ping_cm(); //pemanggilan fungsi library      
  
  return distance;
}

int UltraFront(){
  int distance;
  distance = sonarCenter.ping_cm(); //pemanggilan fungsi library     
  
  return distance;
}
//-------------------End Sensor Ultraasonic------------------------
//-------------------Presentase dan kondisi battery----------------
int batteryPercent(){  
  int volt = analogRead(voltCek);// read the input 
  VOut = (volt * 5.0) / 1024.0;
  VIn = VOut / (R2/(R1+R2));  
  hasil = ((VIn - Vmin) / (Vmax - Vmin)) * 100;
  battery = round(hasil);
  
  if(battery < 100 && battery > 0){
    return battery;     
  }else if (VIn < Vmin){
    return 0;
  }else{
    return 100;
  }
}
//-------------------End Presentase dan kondisi battery----------------
//-------------------kumpulan fungsi kondisi perangkat----------------
void discoPlay(){
  //memberikan peringatan suara bahwa alat tidak terhubung ke hotspot
  currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 > interval1){
    previousMillis1 = currentMillis1;              
    myDFPlayer.play(13); delay(1000);//memutar sound   
  }
}

void batteryLow20(){
  //memberikan peringatan suara bahwa baterai alat tersisa 20%
  currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 > 30000){
    previousMillis1 = currentMillis1;              
    myDFPlayer.play(14); delay(1000);//memutar sound   
  }
}

void batteryLow10(){
  //memberikan peringatan suara bahwa baterai alat tersisa 10%
  currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 > 15000){
    previousMillis1 = currentMillis1;              
    myDFPlayer.play(15); delay(1000);//memutar sound   
  }
}

void batteryLow5(){
  //memberikan peringatan suara bahwa baterai alat tersisa 5%
  currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 > interval1){
    previousMillis1 = currentMillis1;              
    myDFPlayer.play(16); delay(1000);//memutar sound   
  }
}

void batteryHabis(){
  //memberikan peringatan suara bahwa baterai alat habis
  currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 > interval1){
    previousMillis1 = currentMillis1;              
    myDFPlayer.play(17); delay(1000);//memutar sound   
  }
}
//-------------------End kumpulan fungsi kondisi perangkat----------------
//-------------------Mengatur ritme getaran soil sesuai kondisi-----------
void holdVibrate(){
  //mengatur ritme getaran soil
  if(soilValue < 960 && soilValue > 800){ //kondisi basah biasa
    currentMillis2 = millis();
    if(currentMillis2 - previousMillis2 > interval2){
      previousMillis2 = currentMillis2; 
      if(holdSoil == 0){
        holdSoil = 1; //getaran soil hidup               
      }else if(holdSoil == 1){
        holdSoil = 0; //getaran soil mati
      }      
    }//interval waktu
  }else if(soilValue < 800){ //kondisi air menggenang
    holdSoil = 1; 
  }else{ //kondisi kering
    holdSoil = 0; 
  }
}
//------------End Mengatur ritme getaran soil sesuai kondisi-----------
//----------------------Fungsi Utama----------------------------
void mainFunction(){   
  soilValue = analogRead(soilPin);    
  if(UltraFront()<100 && UltraFront()>2){
    digitalWrite(vibFront, HIGH); //vibrate          
  }else{
    digitalWrite(vibFront, LOW); //stop vibrate
  }
  if(UltraLeft()<100 && UltraLeft()>2){
    digitalWrite(vibLeft, HIGH); //vibrate          
  }else{
    digitalWrite(vibLeft, LOW); //stop vibrate          
  }
  if(UltraRight()<100 && UltraRight()>2){
    digitalWrite(vibRight, HIGH); //vibrate          
  }else{
    digitalWrite(vibRight, LOW); //stop vibrate          
  }
  holdVibrate(); //mengatur ritme dari getaran soil moisture
  if(holdSoil == 1){
    digitalWrite(vibSoil, HIGH); //vibrate         
  }else{
    digitalWrite(vibSoil, LOW); //stop vibrate         
  }
  gpsData(); //untuk mendapatkan data GPS
  Button(); //untuk mendapatkan nilai tombol  
}
//-------------------End Fungsi Utama----------------------

void loop() {
  mainFunction(); //fungsi utama untuk pengguna      
  // kirim data ke esp8266
  currentMillis = millis();
  if(currentMillis - previousMillis > interval && connStatus == 1){
    previousMillis = currentMillis;              
    espSerial.print("espFirebase,"+String(gpsStatus)+","+String(statusFinger)+","+dataID+","+String(buttonValue)+","+String(batteryPercent())+",");
    espSerial.print(String(latVar,6)+","+String(lngVar,6)+",");
    espSerial.println(dataPulse);   
    Serial.println("data terkirim ke esp");
  }
  //kumpulan kondisi alat
  if(connStatus == 0){
    discoPlay();
  }else if(batteryPercent() == 20){
    batteryLow20();
  }else if(batteryPercent() == 10){
    batteryLow10();
  }else if(batteryPercent() == 5){
    batteryLow5();
  }else if(batteryPercent() < 5){
    batteryHabis();
  }
  //menerima data dari pulse sensor
  while(pulseSerial.available()){
    pulseSerial.setTimeout(100);
    dataPulse = pulseSerial.readString();          
  } 
  //menerima data kondisi dan perintah dari esp8266
  while(espSerial.available()){
    espSerial.setTimeout(100);
    String d = espSerial.readString();    
    if(d.startsWith("enfinger") && statusFinger != 1){
      d.replace("enfinger","");
      id = d.toInt();      
      enrollFinger();
    }else if(d.startsWith("delfinger") && statusFinger != 2){
      d.replace("delfinger","");
      id = d.toInt();
      deleteFinger();      
    }else if(d.startsWith("done")){
      d.replace("done","");
      statusFinger = d.toInt();
    }else if(d.startsWith("disconnect")){
      d.replace("disconnect","");
      connStatus = d.toInt();
    }else if(d.startsWith("connect")){
      d.replace("connect","");
      myDFPlayer.play(12);  delay(2000);//memutar sound
      connStatus = d.toInt();
    }
  }
}

//----------------------Fungsi upload data GPS--------------------------  
void gpsData(){
  while(gpsSerial.available()>0){
    if (gps.encode(gpsSerial.read())) {
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
  if(gps.location.age()>2000){
    gpsStatus = 0;
  }else if(gps.location.isValid()){
    latVar = gps.location.lat();
    lngVar = gps.location.lng();
    gpsStatus = 1;    
  }else{
    latVar = 0; lngVar = 0;
    gpsStatus = 0;
  }
}
//------------------------End Fungsi Show data GPS------------------------

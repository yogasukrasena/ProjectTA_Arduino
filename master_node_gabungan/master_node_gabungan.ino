#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial fpSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino
SoftwareSerial gpsSerial(0, 2); //rx(D3), tx(D4) untuk menghubungkan dengan gps
SoftwareSerial pulseSerial(13, 15); //rx(D7), tx(D8) untuk menghubungkan dengan pulse

TinyGPSPlus gps;  // The TinyGPS++ object

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
float latVar = 0;
float lngVar = 0;

// id untuk fingerprint
boolean sidikJari = false;
int id;
String dataID;
String separator = ".";

// variables will change:
int buttonState = 0;
int soilValue, fingerID, buttonValue, dataInt;

// variable penerima data dari arduino
String dt[40];
String dataIn;
boolean parsing = false;
int i;

// variable penerima data dari micro
String dt2[40];
String dataIn2;
boolean parsing2 = false;
int l;

//variable setup device
char MacAddress[30];
char ipAddress[40];
char nameDevice[40] = "Device_";
char nameLog[30];

int connStatus, gpsStatus;

//ultrasonic define pin
#define echoFront 10 //S3
#define trigFront 9 //S2
#define echoRight 8 //S1
#define trigRight 11 //SC
#define echoLeft 7 //SO
#define trigLeft 6 //SK

#define soilPin A0 //A0
#define buttonPin 1  //TX
#define buzzer 3  //RX
 
//vibration pin
#define vibFront 16 //D0
#define vibRight 5 //D1
#define vibLeft 4 //D2

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "taproject-53d6c-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBB26FHburuVXXyCf4PbmI5t18oc4j9Qt8"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "yoga@admin.com"
#define USER_PASSWORD "12Yoga12"

#define buzzer 4
/* 4. Define FirebaseESP8266 data object for data sending and receiving */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig data for config data */
FirebaseConfig config;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);

void setup() {
  // Initialize Serial port
  Serial.begin(9600);  
  gpsSerial.begin(9600);
  pulseSerial.begin(115200);
  
  while (!Serial) continue;
  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //connecting to wifi
  while (WiFi.status() != WL_CONNECTED)// while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."
    connStatus = 0;        
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //finger setup
  finger.begin(57600);  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
//  // cek id fingerprint
//  for (int finger = 1; finger < 10; finger++) {
//    downloadFingerprintTemplate(finger);
//  }
//
////  finger.getTemplateCount();
//
//  if (finger.templateCount == 0) {
//    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
//    id = 1;
//    enrollFinger();    
//  }
//  else {
//    Serial.println("Waiting for valid finger...");
//    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");    
//  }
  
  /* 7. Assign the project host and api key (required) */
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;

  /* 8. Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  /* 9. Initialize the library with the autentication data */
  Firebase.begin(&config, &auth);

  /* 10. Enable auto reconnect the WiFi when connection lost */
  Firebase.reconnectWiFi(true);

  //--convert mac address and ip address from string to char--
  String dataMac = WiFi.macAddress();
  String dataIp = WiFi.localIP().toString();
  dataIp.toCharArray(ipAddress, 40);
  dataMac.toCharArray(MacAddress, 40);  
  strcat(nameDevice,MacAddress); //combine hasil konversi kedalam variabel  
  //--End convert mac address and ip address from string to char--
  Serial.println(nameDevice);
  Serial.println(ipAddress);  // Print the IP address
  Serial.println(MacAddress);  // Print the mac address  
  connStatus = 1;
  idAlat();
  setLogTime();

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

//---------------Fungsi cek Setup Koneksi--------------------
void failConnect(){
//Failed?, get the error reason from fbdo      
  Serial.println(fbdo.errorReason());
  if (fbdo.errorReason() == "connection lost"){
    ESP.reset();
  }else if(fbdo.errorReason() == "not connected"){
    connStatus = 2;
  }
}
//---------------End Fungsi cek Setup Koneksi--------------------

//----------------------Cek ID Finger---------------------------------
uint8_t downloadFingerprintTemplate(uint16_t id)
{
  yield();
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

//  // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
//  uint8_t bytesReceived[534]; // 2 data packets
//  memset(bytesReceived, 0xff, 534);
//
//  uint32_t starttime = millis();
//  int i = 0;
//  while (i < 534 && (millis() - starttime) < 20000) {
//      if (fpSerial.available()) {
//          bytesReceived[i++] = fpSerial.read();
//      }
//  }
////  Serial.print(i); Serial.println(" bytes read.");
////  Serial.println("Decoding packet...");
//
//  uint8_t fingerTemplate[512]; // the real template
//  memset(fingerTemplate, 0xff, 512);
//
//  // filtering only the data packets
//  int uindx = 9, index = 0;
//  while (index < 534) {
//      while (index < uindx) ++index;
//      uindx += 256;
//      while (index < uindx) {
//          fingerTemplate[index++] = bytesReceived[index];
//      }
//      uindx += 2;
//      while (index < uindx) ++index;
//      uindx = index + 9;
//  }
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

//---------------Fungsi Upload Setup data Alat--------------------
void idAlat(){
  char name_id[40], ip_add[40], user_name[40], user_keluarga[40];
  strcpy(name_id, nameDevice);
  strcpy(ip_add, nameDevice);
  strcpy(user_name, nameDevice);
  strcpy(user_keluarga, nameDevice);  
  if(Firebase.RTDB.setString(&fbdo,strcat(name_id,"/device_id"), MacAddress)&&
     Firebase.RTDB.setString(&fbdo,strcat(ip_add,"/ip_address"), ipAddress)&&
     Firebase.RTDB.setString(&fbdo,strcat(user_name,"/user_pengguna"), "userPengguna")&&
     Firebase.RTDB.setString(&fbdo,strcat(user_keluarga,"/user_keluarga"), "userKeluarga")){  
    //success
      Serial.println("Set Identias Alat success");
      connStatus = 1;
  }else{
    Serial.print("Error in set idAlat, ");
    failConnect();
  }
}
//---------------End Fungsi Upload Setup data Alat--------------------
//---------------Fungsi Upload Setup data Log Alat--------------------
void setLogTime(){
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int jam = timeClient.getHours();
  int menit = timeClient.getMinutes();  
  int monthDay = ptm->tm_mday;  
  int currentMonth = ptm->tm_mon+1; 
  int currentYear = ptm->tm_year+1900;  
  char currentWaktu[70], currentDate[70];
  char start_nama[40], end_nama[40];
  char tanggal_nama[40], selisih[40];
  char bpm_max[40], bpm_min[40];  
  char spo_max[40], spo_min[40];
   
  sprintf(currentWaktu,"%02d:%02d",jam,menit);
  sprintf(currentDate,"%02d-%02d-%02d",monthDay,currentMonth,currentYear);
  
  sprintf(nameLog,"%s_%s",currentDate,currentWaktu);        
  
  sprintf(start_nama,"%s/log_data/%s/waktu_mulai",nameDevice,nameLog);
  if(Firebase.RTDB.setString(&fbdo, start_nama, currentWaktu)){
    sprintf(tanggal_nama,"%s/log_data/%s/tanggal",nameDevice,nameLog);
    Firebase.RTDB.setString(&fbdo, tanggal_nama, currentDate);
    sprintf(end_nama,"%s/log_data/%s/waktu_berjalan",nameDevice,nameLog);
    Firebase.RTDB.setString(&fbdo, end_nama, currentWaktu);
    sprintf(selisih,"%s/log_data/%s/selisih_waktu",nameDevice,nameLog);
    Firebase.RTDB.setString(&fbdo, selisih, "00:00");
    sprintf(bpm_max,"%s/log_data/%s/bpm_max",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, bpm_max, 0);
    sprintf(bpm_min,"%s/log_data/%s/bpm_min",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, bpm_min, 0);
    sprintf(spo_max,"%s/log_data/%s/spo_max",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, spo_max, 0);
    sprintf(spo_min,"%s/log_data/%s/spo_min",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, spo_min, 0);

    Serial.println("Set Log Sukses di Upload");    
    connStatus = 1;    
  }else{
    Serial.print("Error in up Log Data, ");
    failConnect();    
  }
}
//---------------End Fungsi Upload Setup data Log Alat--------------------
//---------------Fungsi Upload Setup data Flag------------------------
void flagStatus(int buttonStatus){
  char status_device[40], status_gps[40], status_fingger[40], status_button[40];
  strcpy(status_device,nameDevice);
  strcpy(status_gps,nameDevice);
  strcpy(status_fingger,nameDevice);
  strcpy(status_button,nameDevice);  
  if(Firebase.RTDB.setInt(&fbdo,strcat(status_device,"/flag_status/status_device"), connStatus)&&
     Firebase.RTDB.setInt(&fbdo,strcat(status_gps,"/flag_status/status_gps"), gpsStatus)&&
     Firebase.RTDB.setDouble(&fbdo,strcat(status_fingger,"/flag_status/status_fingger"), 0)&&
     Firebase.RTDB.setInt(&fbdo,strcat(status_button,"/flag_status/status_button"), buttonStatus)){
      //success
      Serial.println("Data Flag Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in set flag, ");
    failConnect();
  }
}
//---------------End Fungsi Upload Setup data Flag---------
//---------------Fungsi Upload data GPS--------------------
void dataGPS(float gpsLat, float gpsLng){
  char lat_value[30], long_value[30];
  strcpy(lat_value,nameDevice);
  strcpy(long_value,nameDevice);  
  //data lokasi
  if(Firebase.RTDB.setDouble(&fbdo, strcat(lat_value,"/raw_data/GPS_Lat"), gpsLat)&&
     Firebase.RTDB.setDouble(&fbdo, strcat(long_value,"/raw_data/GPS_Long"), gpsLng)){
      Serial.println("Data GPS Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in up GPS, ");
    failConnect();
  }
}
//---------------End Fungsi Upload data GPS------------------
//---------------Fungsi Upload data Pulse--------------------
void dataPulse(int bpm, int spo2){
  char bpm_value[30], spo2_value[30];
  strcpy(bpm_value,nameDevice);
  strcpy(spo2_value,nameDevice); 
  //data lokasi
  if(Firebase.RTDB.setInt(&fbdo, strcat(bpm_value,"/raw_data/bpm_level"), bpm)&&
     Firebase.RTDB.setInt(&fbdo, strcat(spo2_value,"/raw_data/spo2_level"), spo2)){           
      Serial.print("Data pulse Sukses di Upload");
      Serial.println(bpm);
      connStatus = 1;
  }else{
    Serial.print("Error in up GPS, ");
    failConnect();
  }
}
//---------------End Fungsi Upload data Pulse------------------
//-----------------Fungsi waktu dari Ntp-----------------------
void realTime(){
  timeClient.update();  
  int jam = timeClient.getHours();
  int menit = timeClient.getMinutes();
  char currentWaktu[40];  
  sprintf(currentWaktu,"%02d:%02d",jam,menit);  
  
  Serial.print("Waktu :");
  Serial.println(currentWaktu);  
  dataTime(currentWaktu);
}
//--------------End Fungsi waktu dari Ntp---------------------
//---------------Fungsi Upload data realTime------------------
void dataTime(String waktu){
  char waktu_field[50];  
  sprintf(waktu_field,"%s/log_data/%s/waktu_berjalan",nameDevice,nameLog);  
  if(Firebase.RTDB.setString(&fbdo, waktu_field, waktu)){
    Serial.println("Data runtime Sukses di Upload");
    connStatus = 1;
  }else{
    Serial.print("Error in up data waktu, ");
    failConnect();
  }
}
//--------------End Fungsi Upload data realTime--------------
//------------Fungsi Setting Upload data alat----------------
void deviceData(int battery_value, String fingger_data){
  char data_fingger[60], baterai_level[40];  
  strcpy(baterai_level,nameDevice);
  strcpy(data_fingger,nameDevice);    
  if(Firebase.RTDB.setInt(&fbdo, strcat(baterai_level,"/raw_data/battery_level"), battery_value)&&
     Firebase.RTDB.setString(&fbdo, strcat(data_fingger,"/raw_data/fingger_data"), fingger_data)){      
      Serial.println("Data Arduino Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in up Arduino Data, ");
    failConnect();
  }  
}
//------------End Fungsi Setting Upload data alat----------------
//----------------Fungsi Upload data pulse-----------------------
void dataLogPulse(int bpm1, int bpm2, int spo1, int spo2){
  char bpm_max[60], bpm_min[50];
  char spo_max[50], spo_min[50];   

  sprintf(bpm_max,"%s/log_data/%s/bpm_max",nameDevice,nameLog);
  if(Firebase.RTDB.setInt(&fbdo, bpm_max, bpm1)){
    sprintf(bpm_min,"%s/log_data/%s/bpm_min",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, bpm_min, bpm2);
    sprintf(spo_max,"%s/log_data/%s/spo_max",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, spo_max, spo1);
    sprintf(spo_min,"%s/log_data/%s/spo_min",nameDevice,nameLog);
    Firebase.RTDB.setInt(&fbdo, spo_min, spo2);
    
    Serial.println("Data log pulse Sukses di Upload");    
    connStatus = 1;
  }else{
    Serial.print("Error in up Arduino Data, ");
    failConnect();
  }
}
//-------------End Fungsi Upload data pulse------------------------
//------------Fungsi Read data Setting Fingger Alat----------------
//void finggerSetup(){
//  char status_fingger[40];
//  strcpy(status_fingger,nameDevice);
//  strcat(status_fingger,"/flag_status/status_fingger");
//  if(Firebase.RTDB.get(&fbdo,status_fingger)){
//    if(fbdo.stringData().startsWith("enfinger")){          
//      wifiSerial.println(fbdo.stringData());      
//      Serial.println("enroll finger");
//      delay(1000);
//      Firebase.RTDB.setDouble(&fbdo,status_fingger, 0);               
//    }else if(fbdo.stringData().startsWith("delfinger")){          
//      wifiSerial.println(fbdo.stringData());      
//      Serial.println("delete finger");
//      delay(1000);
//      Firebase.RTDB.setDouble(&fbdo,status_fingger, 0);               
//    }  
//  }else{
//    Serial.print("Error in get Fingger, ");
//    failConnect();
//  } 
//}
//----------End Fungsi Read data Setting Fingger Alat-------------
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
  fingerID = getFingerprintID();    
  while(sidikJari){  
    gpsData();
    realTime();
    mainFunction();    
    //--Read data fingger dari firebase dan mengirimkan ke arduino--
//    finggerSetup();
    //--End Read data dari firebase dan mengirimkan ke arduino--
    //--Read data parsing dari arduino--
    while(pulseSerial.available()>0){
      char inChar2 = (char)pulseSerial.read();
      dataIn2 += inChar2;
      if (inChar2 == '\n'){
        parsing2 = true;
      }
    }           
    if(parsing2){
      parsingData2();    
      parsing2=false;
      dataIn2="";       
    }
    //--End Read data parsing dari arduino--
  }
}

//----------------------Fungsi PARSING DATA---------------------------  
void parsingData2(){
 int k=1;    
 //--PARSING SELURUH DATA--
  dt2[k]="";
  for(l=1;l<dataIn2.length();l++){
    if ((dataIn2[l] == ':')){
      k++;
      dt2[k]="";
    }
    else{
      dt2[k] = dt2[k] + dataIn2[l];
    }
  }
  //--PRINT DATA YANG TELAH DI PARSING--
  Serial.println("--------------Parsing Data----------------");
  Serial.print("Data 1 (flag) : ");
  Serial.println(dt2[1]);
  Serial.print("Data 2 (BPM) : ");
  Serial.println(dt2[2].toInt());
  Serial.print("Data 3 (SPO2) : ");
  Serial.println(dt2[3].toInt());  
  Serial.print("Data 4 (maxBpm) : ");
  Serial.println(dt2[4].toInt());
  Serial.print("Data 5 (minBpm) : ");
  Serial.println(dt2[5].toInt());
  Serial.print("Data 6 (maxSpo) : ");
  Serial.println(dt2[6].toInt());   
  Serial.print("Data 7 (minSpo) : ");
  Serial.println(dt2[7].toInt());    
  //--KIRIM DATA KE FIREBASE--
//  dataPulse(dt2[2].toInt(),dt2[3].toInt()); 
//  dataLogPulse(dt2[4].toInt(),dt2[5].toInt(),dt2[6].toInt(),dt2[7].toInt());
}
//----------------------End Fungsi PARSING DATA---------------------------  
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
  if(gps.location.isValid()){
    latVar = gps.location.lat();
    lngVar = gps.location.lng();
    
    Serial.print("Location = ");
    Serial.print(latVar, 9);
    Serial.print(",");
    Serial.println(lngVar, 9);
    gpsStatus = 1;
//    dataGPS(latVar,lngVar);
  }else{
    Serial.println("Lokasi Belum Terbaca");
    gpsStatus = 0;
  }
}
//------------------------End Fungsi Show data GPS------------------------

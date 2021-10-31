#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool onProcess = false;
bool onRequest = false;

int modeESP8266 = 0;

const char* ssid = "redmi"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi

//variable setup device
String MacAddress = WiFi.macAddress();
String nameDevice = "Device_"+MacAddress;
String ipAddress = WiFi.localIP().toString();
String nameLog;
String hasil_parsing;
//variabel parsing
String dataIn[100], s;

int finger, connStatus;

unsigned long senddataInPrevMillis = 0;

unsigned long count = 0;

/* 1. Define the WiFi credentials */
#define WIFI_SSID "redmi"
#define WIFI_PASSWORD "shittman"

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "taproject-53d6c-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "jXcTzp4PKeQJwVh3AFFHIobwOgvcyJkxTcqDvLmH"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

void setup() {
  // Initialize Serial port
  Serial.begin(9600); 
  Serial.println();

  connectESP();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the certificate file (optional) */
  //config.cert.file = "/cert.cer";
  //config.cert.file_storage = StorageType::FLASH;

  /* Assign the database URL and database secret(required) */
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
      
  timeClient.begin();
  timeClient.setTimeOffset(28800);  

  setFirebase();   
}

void disconnectESP(){
  WiFi.setAutoConnect( true );
  WiFi.setAutoReconnect( true );
  WiFi.disconnect( false );
  WiFi.softAPdisconnect( false );
  modeESP8266 = 0;    
}
//------------------Konek kedalam jaringan--------------------------
//----------(jika device baru menyala dan disconnect sebelumnya)-----
void connectESP(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  ipAddress = WiFi.localIP().toString();
  modeESP8266 = 1;
}
//----------------Cek koneksi yang terhubung-------------------------
void checkConn(){
  if( modeESP8266>0 && WiFi.localIP().toString()=="(IP unset)"){
    connectESP();
  }
}

//------------------fungsi setup firebase untuk device---------------
void setFirebase(){
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
   
  sprintf(currentWaktu,"%02d:%02d",jam,menit);
  sprintf(currentDate,"%02d-%02d-%02d",monthDay,currentMonth,currentYear);

  nameLog = nameDevice+"/log_data/"+String(currentDate)+"_"+String(currentWaktu);
  
  if(Firebase.setString(fbdo,nameDevice+"/device_id", WiFi.macAddress())){
    if(Firebase.getString(fbdo,nameDevice+"/user_pengguna")){
      if(fbdo.stringData()=="null"){
        Firebase.setString(fbdo,nameDevice+"/user_pengguna", "userPengguna");
      }else{
        Serial.println("data Pengguna Tersedia");
      }
    }
    if(Firebase.getString(fbdo,nameDevice+"/user_keluarga")){
      if(fbdo.stringData()=="null"){
        Firebase.setString(fbdo,nameDevice+"/user_keluarga", "userKeluarga");
      }else{
        Serial.println("data Keluarga Tersedia");
      }
    }
    if(Firebase.getDouble(fbdo,nameDevice+"/raw_data/GPS_Lat")){
      if(fbdo.doubleData()==0){
        Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Lat", 0);
      }else{
        Serial.println("data GPS Lat Tersedia");
      }
    }
    if(Firebase.getDouble(fbdo,nameDevice+"/raw_data/GPS_Long")){
      if(fbdo.doubleData()==0){
        Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Long", 0);
      }else{
        Serial.println("data GPS Long Tersedia");
      }
    }
    
    Firebase.setString(fbdo,nameDevice+"/ip_address", ipAddress);
    //set flag dataIn    
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_device", connStatus);
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_gps", 0);
    Firebase.setString(fbdo,nameDevice+"/flag_status/status_finger", "noneAction");
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_button", 0);
    //set raw dataIn       
    Firebase.setInt(fbdo, nameDevice+"/raw_data/bpm_level", 0);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/spo2_level", 0);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/battery_level", 0);
    Firebase.setString(fbdo, nameDevice+"/raw_data/finger_data", "0");
    //set log dataIn
    Firebase.setString(fbdo,nameLog+"/waktu_mulai", currentWaktu);
    Firebase.setString(fbdo,nameLog+"/tanggal", currentDate);    
    Firebase.setString(fbdo,nameLog+"/waktu_berjalan", currentWaktu);    
    Firebase.setString(fbdo,nameLog+"/selisih", "00:00");    
    Firebase.setInt(fbdo,nameLog+"/bpm_max", 0);    
    Firebase.setInt(fbdo,nameLog+"/bpm_min", 0);
    Firebase.setInt(fbdo,nameLog+"/spo_max", 0);
    Firebase.setInt(fbdo,nameLog+"/spo_min", 0);
    Serial.println("Set Firebase Sukses");    
    connStatus = 1;    
  }else{
    Serial.print("Error in up Log data, ");
    Serial.println(fbdo.errorReason());   
  }
}

//------------------End fungsi setup firebase untuk device---------------
//---------------Fungsi Upload Setup dataIn Flag------------------------
void flagStatus(int buttonStatus, int gpsStatus){
  if(Firebase.setInt(fbdo,nameDevice+"/flag_status/status_device", connStatus)&&
     Firebase.setInt(fbdo,nameDevice+"/flag_status/status_gps", gpsStatus)&&     
     Firebase.setInt(fbdo,nameDevice+"/flag_status/status_button", buttonStatus)){
      //success
      Serial.println("data Flag Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in set flag, ");
    Serial.println(fbdo.errorReason());
  }
}
//---------------End Fungsi Upload Setup dataIn Flag-----------------
//-----------------fungsi kirim dataIn raw---------------------------
void dataRaw(int bpm, int spo2, int battery, String finger){
  if(Firebase.setInt(fbdo, nameDevice+"/raw_data/bpm_level",bpm)){
    Firebase.setInt(fbdo, nameDevice+"/raw_data/spo2_level",spo2);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/battery_level",battery);
    Firebase.setString(fbdo, nameDevice+"/raw_data/finger_data",finger);  
    
    Serial.println("data Raw Sukses di Upload");
    connStatus = 1;
  }else{
    Serial.print("Error in up raw data, ");
    Serial.println(fbdo.errorReason());
  }
}
//-----------------End fungsi kirim dataIn raw-----------------------
//-----------------fungsi kirim dataIn log---------------------------
void dataLog(int bpmMax, int bpmMin, int spoMax, int spoMin){
  timeClient.update();  
  int jam = timeClient.getHours();
  int menit = timeClient.getMinutes();
  char currentWaktu[40];  
  sprintf(currentWaktu,"%02d:%02d",jam,menit);  
  if(Firebase.setString(fbdo,nameLog+"/waktu_berjalan", currentWaktu)){
    Firebase.setInt(fbdo,nameLog+"/bpm_max", bpmMax);    
    Firebase.setInt(fbdo,nameLog+"/bpm_min", bpmMin);
    Firebase.setInt(fbdo,nameLog+"/spo_max", spoMax);
    Firebase.setInt(fbdo,nameLog+"/spo_min", spoMin);
    
    Serial.println("data Log Sukses di Upload");
    connStatus = 1;  
  }else{
    Serial.print("Error in up Log, ");
    Serial.println(fbdo.errorReason());
  }
}
//------------------End fungsi kirim data log----------------------
void dataGps(double gpsLat, double gpsLong){
  if(Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Lat",gpsLat)){
    Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Long",gpsLong);

    Serial.println("data GPS Sukses di Upload");
    connStatus = 1;  
  }else{
    Serial.print("Error in up Log, ");
    Serial.println(fbdo.errorReason());
  }  
}

//------------Fungsi Read dataIn Setting Fingger Alat----------------
void finggerSetup(){ 
  if(Firebase.getString(fbdo,nameDevice+"/flag_status/status_finger")){
    if(fbdo.stringData().startsWith("enfinger")){          
      Serial.println(fbdo.stringData());      
//      Serial.println("enroll finger");
      delay(1000);      
    }else if(fbdo.stringData().startsWith("delfinger")){          
      Serial.println(fbdo.stringData());      
//      Serial.println("delete finger");
      delay(1000);      
    }else if(fbdo.stringData().startsWith("cancel")){
      Serial.println(fbdo.stringData());
      delay(1000);      
    }else if(fbdo.stringData().startsWith("done")){
      Serial.println(fbdo.stringData());
      delay(1000);
    }
  }else{
    Serial.print("Error in get Fingger, ");
    Serial.println(fbdo.errorReason());
  } 
}
//----------End Fungsi Read dataIn Setting Fingger Alat-------------

void loop() {  
   if(Serial.available() && !onProcess){
    onProcess = true;
    s = Serial.readStringUntil('\n');
    
    // PARSE SERIAL INPUT -------------------------------------
    // Remove new line char and add a comma to last line
    while(s.charAt(s.length()-1)=='\n'){
      s = s.substring(0,s.length()-1);  
    }
    if(s.charAt(s.length()-1)==','){
      s = s + ",";
    }
    //
    s = s.substring(0,s.length()-1)+", ";    
    int r = 0;
    int t = 0;
    for(int i=0;i<s.length();i++){      
      if(s.charAt(i)==','){
        dataIn[t] = s.substring(r,i);
        r = i+1;
        t++;      
      }
    }  
    // PARSE SERIAL INPUT -------------------------------------

    // GET SIMPLE STATUS --------------------------------------
    if(dataIn[0]=="espStatus"){
      // 0 = NOT CONNECTED
      // 1 = CONNECTED TO AP
      // 2 = AS AP
      checkConn();
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // GET SIMPLE STATUS --------------------------------------

    // GET SET/GET GPIO STATUS --------------------------------
    if(dataIn[0]=="espGPIO0"){
      if(dataIn[1]=="0" || dataIn[1]=="1" || dataIn[1]=="LOW" || dataIn[1]=="HIGH"){
        // SET GPIO STATUS
        pinMode(0,OUTPUT);
        digitalWrite(0,dataIn[1].toInt());
        Serial.print("espGPIO0,");Serial.println(digitalRead(0));        
      }else{
        // GET GPIO STATUS
        pinMode(0,INPUT);
        Serial.print("espGPIO0,");Serial.println(digitalRead(0));       
      }
    }
    if(dataIn[0]=="espGPIO2"){
      if(dataIn[1]=="0" || dataIn[1]=="1" || dataIn[1]=="LOW" || dataIn[1]=="HIGH"){
        // SET GPIO STATUS
        pinMode(2,OUTPUT);
        digitalWrite(2,dataIn[1].toInt());
        Serial.print("espGPIO2,");Serial.println(digitalRead(2));
      }else{
        // GET GPIO STATUS
        pinMode(2,INPUT);
        Serial.print("espGPIO2,");Serial.println(digitalRead(2));        
      }
    }
    // GET SET/GET GPIO STATUS --------------------------------

    // DISCONNECT ESP -----------------------------------------
    if(dataIn[0]=="espDisconnect"){
      disconnectESP();
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // DISCONNECT ESP -----------------------------------------

    // GET ALL STATUS -----------------------------------------
    if(dataIn[0]=="espGetStatus"){      
      Serial.println();
      checkConn();
      if(WiFi.status()==WL_CONNECTED){
        Serial.println("Status : WL_CONNECTED");
      }
      if(WiFi.status()==WL_NO_SSID_AVAIL){
        Serial.println("Status : WL_NO_SSID_AVAIL");
      }
      if(WiFi.status()==WL_CONNECT_FAILED){
        Serial.println("Status : WL_CONNECT_FAILED");
      }
      if(WiFi.status()==WL_IDLE_STATUS){
        Serial.println("Status : WL_IDLE_STATUS");
      }
      if(WiFi.status()==WL_DISCONNECTED){
        Serial.println("Status : WL_DISCONNECTED");
      }
      if(WiFi.status()==-1){
        Serial.println("Status : TIMEOUT");
      }
      if(modeESP8266==1){
        if(WiFi.status()==WL_CONNECTED){
          Serial.print("Connected to    : ");Serial.println(WiFi.SSID());
          Serial.print("Signal strength : ");Serial.println(WiFi.RSSI());
          Serial.print("Local IP        : ");Serial.println(WiFi.localIP().toString());
          Serial.print("Hostname        : ");Serial.println(WiFi.hostname());
          
          Serial.print("Mac address     : ");Serial.println(WiFi.macAddress());
          Serial.print("Subnet mask     : ");Serial.println(WiFi.subnetMask());
          Serial.print("DNS             : ");Serial.print(WiFi.dnsIP());Serial.print(",");Serial.println(WiFi.dnsIP(1));
        }
      }
      if(modeESP8266==2){
        Serial.print("AP Mode with local IP : ");Serial.println(WiFi.softAPIP());
        Serial.print("Mac address           : ");Serial.println(WiFi.softAPmacAddress());
        Serial.print("Num of client AP      : ");Serial.println(WiFi.softAPgetStationNum());
      }      
      Serial.println();
    }
    // GET ALL STATUS -----------------------------------------

    // HELP ---------------------------------------------------
    if(dataIn[0]=="espHelp"){
      Serial.println();
      Serial.println("Connect to AP   : espClientMode,ssid,password");
      Serial.println("Set as AP Mode  : espApMode,ssid,password");
      Serial.println("Get  status     : espStatus or espGetStatus");
      Serial.println("Request http    : espHttp,GET/POST,url,payload");
      Serial.println("Disconnect all  : espDisconnect");
      Serial.println("SET GPIO0/2     : espGPIO0/2,0/1/LOW/HIGH");
      Serial.println("GET GPIO0/2     : espGPIO0/2");
      Serial.println();
    }
    // HELP ---------------------------------------------------

    // CONNECT ESP8266 TO AN ACCESS POINT ---------------------
    // url:https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html
    if(dataIn[0]=="espClientMode"){      
      disconnectESP();
      WiFi.begin(dataIn[1], dataIn[2]);    
      int i=0;
      while (WiFi.status() != WL_CONNECTED)
      {
        Serial.print(".");
        i++;
        delay(500);
        if(i>20){
          // break after 10s waiting
          modeESP8266 = 0;
          Serial.println("Skip connect");
          break;
        }
      }
      if(WiFi.status()==WL_CONNECTED){
        modeESP8266 = 1;          
      }        
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // CONNECT ESP8266 TO AN ACCESS POINT ---------------------

    // SET ESP8266 AS AN ACCESS POINT -------------------------
    // url : https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
    if(dataIn[0]=="espApMode"){      
      disconnectESP();
      IPAddress local_IP(192,168,99,1);      
      IPAddress gateway(192,168,99,1); 
      IPAddress subnet(255,255,255,0);
      WiFi.mode( WIFI_AP);
      if(WiFi.softAPConfig(local_IP, gateway, subnet)){
        modeESP8266 = 2;
      }else{
        modeESP8266 = 0;
      }
      if(modeESP8266==2 && WiFi.softAP(dataIn[1], dataIn[2],false,8)){
        modeESP8266 = 2;
        WiFi.setAutoConnect( true );
        WiFi.setAutoReconnect( true );
      }else{
        modeESP8266 = 0;
      }
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // SET ESP8266 AS AN ACCESS POINT -------------------------
    // ESP8266 mode client mengirimkan data device ke dalam firebase
    if(dataIn[0]=="espFirebase"){
      checkConn();
      if(modeESP8266>0){
        flagStatus(dataIn[3].toInt(),dataIn[1].toInt());
        dataRaw(dataIn[7].toInt(),dataIn[8].toInt(),dataIn[4].toInt(),dataIn[2]);
        dataLog(dataIn[9].toInt(),dataIn[10].toInt(),dataIn[11].toInt(),dataIn[12].toInt());
        finggerSetup();
        if(dataIn[1].toInt()>0){         
          dataGps(dataIn[5].toDouble(), dataIn[6].toDouble());         
        }        
      }
    }
    // ESP8266 mode client menjalankan perintah scan sidik jari
    if(dataIn[0]=="rollFinger"){
      checkConn();
      if(modeESP8266>0){
        finggerSetup();        
        Firebase.setString(fbdo, nameDevice+"/flag_status/status_finger",dataIn[1]);
      }
    }
    onProcess = false;
  }   
}

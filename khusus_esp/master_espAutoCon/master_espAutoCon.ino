#include <Firebase_ESP_Client.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
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
String nameDevice = "Device_asdajdhjkahdkjasd";
String ipAddress = WiFi.localIP().toString();
String nameLog;
String hasil_parsing;
//variabel parsing
String dataIn[100], s;

int finger, connStatus;

unsigned long senddataInPrevMillis = 0;

unsigned long count = 0;
/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "taproject-53d6c-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBB26FHburuVXXyCf4PbmI5t18oc4j9Qt8"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "yoga@admin.com"
#define USER_PASSWORD "12Yoga12"

#define buzzer 4
/* 4. Define FirebaseESP8266 dataIn object for dataIn sending and receiving */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth dataIn for authentication dataIn */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig dataIn for config dataIn */
FirebaseConfig config;

void setup() {
  // Initialize Serial port
  Serial.begin(9600); 
  Serial.println();

  connectESP();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);      
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = FIREBASE_HOST;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.dataInbase_url = dataInBASE_URL;
  //config.signer.tokens.legacy_token = "<dataInbase secret>";

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);                 
      
  timeClient.begin();
  timeClient.setTimeOffset(28800);  

  setFirebase();
  
  //  disconnectESP();  
}

void disconnectESP(){
  WiFi.setAutoConnect( true );
  WiFi.setAutoReconnect( true );
  WiFi.disconnect( false );
  WiFi.softAPdisconnect( false );
  modeESP8266 = 0;    
}

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
  Serial.println();
  modeESP8266 = 1;
}

void checkConn(){
  if( modeESP8266>0 && WiFi.localIP().toString()=="(IP unset)"){
    connectESP();
  }
}

//---------------Fungsi cek Setup Koneksi--------------------
//void failConnect(){
////Failed?, get the error reason from fbdo      
//  Serial.println(fbdo.errorReason());
//  if (fbdo.errorReason() == "connection lost"){
//    ESP.reset();
//  }else if(fbdo.errorReason() == "not connected"){
//    connStatus = 2;
//  }
//}
//---------------End Fungsi cek Setup Koneksi---------------------
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
  
  if(Firebase.RTDB.setString(&fbdo,nameDevice+"/device_id", WiFi.macAddress())){
    if(Firebase.RTDB.get(&fbdo,nameDevice+"/user_pengguna")){
      if(fbdo.stringData()=="null"){
        Firebase.RTDB.setString(&fbdo,nameDevice+"/user_pengguna", "userPengguna");
      }else{
        Serial.println("data Pengguna Tersedia");
      }
    }
    if(Firebase.RTDB.get(&fbdo,nameDevice+"/user_keluarga")){
      if(fbdo.stringData()=="null"){
        Firebase.RTDB.setString(&fbdo,nameDevice+"/user_keluarga", "userKeluarga");
      }else{
        Serial.println("data Keluarga Tersedia");
      }
    }
    if(Firebase.RTDB.get(&fbdo,nameDevice+"/raw_data/GPS_Lat")){
      if(fbdo.doubleData()==0){
        Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Lat", 0);
      }else{
        Serial.println("data GPS Lat Tersedia");
      }
    }
    if(Firebase.RTDB.get(&fbdo,nameDevice+"/raw_data/GPS_Long")){
      if(fbdo.doubleData()==0){
        Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Long", 0);
      }else{
        Serial.println("data GPS Long Tersedia");
      }
    }
    
    Firebase.RTDB.setString(&fbdo,nameDevice+"/ip_address", ipAddress);
    //set flag dataIn    
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_device", connStatus);
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_gps", 0);
    Firebase.RTDB.setDouble(&fbdo,nameDevice+"/flag_status/status_fingger", 0);
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_button", 0);
    //set raw dataIn       
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/bpm_level", 0);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/spo2_level", 0);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/battery_level", 0);
    Firebase.RTDB.setString(&fbdo, nameDevice+"/raw_data/fingger_data", "0");
    //set log dataIn
    Firebase.RTDB.setString(&fbdo,nameLog+"/waktu_mulai", currentWaktu);
    Firebase.RTDB.setString(&fbdo,nameLog+"/tanggal", currentDate);    
    Firebase.RTDB.setString(&fbdo,nameLog+"/waktu_berjalan", currentWaktu);    
    Firebase.RTDB.setString(&fbdo,nameLog+"/selisih", "00:00");    
    Firebase.RTDB.setInt(&fbdo,nameLog+"/bpm_max", 0);    
    Firebase.RTDB.setInt(&fbdo,nameLog+"/bpm_min", 0);
    Firebase.RTDB.setInt(&fbdo,nameLog+"/spo_max", 0);
    Firebase.RTDB.setInt(&fbdo,nameLog+"/spo_min", 0);
    Serial.println("Set Firebase Sukses");    
    connStatus = 1;    
  }else{
    Serial.print("Error in up Log data, ");
    Serial.println(fbdo.errorReason());   
  }
}

//---------------Fungsi Upload Setup dataIn Flag------------------------
void flagStatus(int buttonStatus, int gpsStatus){
  if(Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_device", connStatus)&&
     Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_gps", gpsStatus)&&     
     Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_button", buttonStatus)){
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
  if(Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/bpm_level",bpm)){
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/spo2_level",spo2);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/battery_level",battery);
    Firebase.RTDB.setString(&fbdo, nameDevice+"/raw_data/fingger_data",finger);  
    
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
  if(Firebase.RTDB.setString(&fbdo,nameLog+"/waktu_berjalan", currentWaktu)){
    Firebase.RTDB.setInt(&fbdo,nameLog+"/bpm_max", bpmMax);    
    Firebase.RTDB.setInt(&fbdo,nameLog+"/bpm_min", bpmMin);
    Firebase.RTDB.setInt(&fbdo,nameLog+"/spo_max", spoMax);
    Firebase.RTDB.setInt(&fbdo,nameLog+"/spo_min", spoMin);
    
    Serial.println("data Log Sukses di Upload");
    connStatus = 1;  
  }else{
    Serial.print("Error in up Log, ");
    Serial.println(fbdo.errorReason());
  }
}
//------------------End fungsi kirim data log----------------------
void dataGps(double gpsLat, double gpsLong){
  if(Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Lat",gpsLat)){
    Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Long",gpsLong);

    Serial.println("data GPS Sukses di Upload");
    connStatus = 1;  
  }else{
    Serial.print("Error in up Log, ");
    Serial.println(fbdo.errorReason());
  }  
}

//------------Fungsi Read dataIn Setting Fingger Alat----------------
void finggerSetup(){ 
  if(Firebase.RTDB.get(&fbdo,nameDevice+"/flag_status/status_fingger")){
    if(fbdo.stringData().startsWith("enfinger")){          
      Serial.println(fbdo.stringData());      
//      Serial.println("enroll finger");
      delay(1000);
      Firebase.RTDB.setDouble(&fbdo,nameDevice+"/flag_status/status_fingger", 0);               
    }else if(fbdo.stringData().startsWith("delfinger")){          
      Serial.println(fbdo.stringData());      
//      Serial.println("delete finger");
      delay(1000);
      Firebase.RTDB.setDouble(&fbdo,nameDevice+"/flag_status/status_fingger", 0);
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

    if(dataIn[0]=="espFirebase"){
      checkConn();
      if(modeESP8266>0){
        flagStatus(dataIn[3].toInt(),dataIn[1].toInt());
        dataRaw(dataIn[7].toInt(),dataIn[8].toInt(),dataIn[4].toInt(),dataIn[2]);
        dataLog(dataIn[9].toInt(),dataIn[10].toInt(),dataIn[11].toInt(),dataIn[12].toInt());
        if(dataIn[1].toInt()>0){         
          dataGps(dataIn[5].toDouble(), dataIn[6].toDouble());
          Serial.println(dataIn[5].toDouble(),6);
          Serial.println(dataIn[6].toDouble(),6);
        }        
      }
    }        
    onProcess = false;
  }   
}

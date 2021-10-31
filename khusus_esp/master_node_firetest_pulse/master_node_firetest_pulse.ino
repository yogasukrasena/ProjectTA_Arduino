#include <Firebase_ESP_Client.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MAX30100_PulseOximeter.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool onProcess = false;
bool onRequest = false;

int modeESP8266 = 0;

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
double latVar = 0;
double lngVar = 0;

//variable setup device
String MacAddress = WiFi.macAddress();
String nameDevice = "Device_asdajdhjkahdkjasd";
String ipAddress;
String nameLog;

int finger, connStatus, gpsStatus;

//pulse sensor
#define REPORTING_PERIOD_MS     1000
 
PulseOximeter pox;

uint32_t tsLastReport = 0;
long startMillis, currentMillis;
const long period = 20000;
int BPM, SPO2;
int maxBpm, minBpm;
int maxSpo, minSpo;
int count;
//pulse sensor

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

void setup() {
  // Initialize Serial port
  Serial.begin(115200); 
  Serial.println();  
        
  timeClient.begin();
  timeClient.setTimeOffset(28800);

  WiFi.begin(ssid, password); //connecting to wifi
  while (WiFi.status() != WL_CONNECTED)// while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."          
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  
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

//  disconnectESP();  

  ipAddress = WiFi.localIP().toString();    
  Serial.println(ipAddress);
  connStatus = 1;

  if(Firebase.RTDB.setInt(&fbdo,"test/hasil", 1)){
    Serial.println("Upload set data berhasil");
  }else{
    Serial.print("Error in up data, ");
    Serial.println(fbdo.errorReason());    
  } 
  
  //pulse sensor  
  Serial.print("Initializing pulse oximeter..");
//   Initialize the PulseOximeter instance
//   Failures are generally due to an improper I2C wiring, missing power supply
//   or wrong target chip  
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");      
  }
  //pulse sensor      
//  setFirebase();
}

void disconnectESP(){
  WiFi.setAutoConnect( false );
  WiFi.setAutoReconnect( false );
  WiFi.disconnect( true );
  WiFi.softAPdisconnect( true );
  modeESP8266 = 0;    
}

void checkConn(){
  if( modeESP8266>0 && WiFi.localIP().toString()=="(IP unset)"){
    disconnectESP();
  }
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
        Serial.println("Data Pengguna Tersedia");
      }
    }
    if(Firebase.RTDB.get(&fbdo,nameDevice+"/user_keluarga")){
      if(fbdo.stringData()=="null"){
        Firebase.RTDB.setString(&fbdo,nameDevice+"/user_keluarga", "userKeluarga");
      }else{
        Serial.println("Data Keluarga Tersedia");
      }
    }
    Firebase.RTDB.setString(&fbdo,nameDevice+"/ip_address", ipAddress);
    //set flag data    
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_device", connStatus);
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_gps", gpsStatus);
    Firebase.RTDB.setDouble(&fbdo,nameDevice+"/flag_status/status_fingger", 0);
    Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_button", 0);
    //set raw data
    Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Lat", 0);
    Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Long", 0);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/bpm_level", 0);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/spo2_level", 0);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/battery_level", 0);
    Firebase.RTDB.setString(&fbdo, nameDevice+"/raw_data/fingger_data", "0");
    //set log data
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
    Serial.print("Error in up Log Data, ");
    failConnect();    
  }
}

//---------------Fungsi Upload Setup data Flag------------------------
void flagStatus(int buttonStatus){
  if(Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_device", connStatus)&&
     Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_gps", gpsStatus)&&     
     Firebase.RTDB.setInt(&fbdo,nameDevice+"/flag_status/status_button", buttonStatus)){
      //success
      Serial.println("Data Flag Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in set flag, ");
    failConnect();
  }
}
//---------------End Fungsi Upload Setup data Flag-----------------
//-----------------fungsi kirim data raw---------------------------
void dataRaw(double gpsLat, double gpsLong, int bpm, int spo2, int battery, String finger){
  if(Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Lat",gpsLat)){
    Firebase.RTDB.setDouble(&fbdo, nameDevice+"/raw_data/GPS_Long",gpsLong);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/bpm_level",bpm);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/spo2_level",spo2);
    Firebase.RTDB.setInt(&fbdo, nameDevice+"/raw_data/battery_level",battery);
    Firebase.RTDB.setString(&fbdo, nameDevice+"/raw_data/fingger_data",finger);  
    
    Serial.println("Data raw Sukses di Upload");
    connStatus = 1;
  }else{
    Serial.print("Error in up raw data, ");
    failConnect();
  }
}
//-----------------End fungsi kirim data raw-----------------------
//-----------------fungsi kirim data log---------------------------
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
    
    Serial.println("Data log Sukses di Upload");
    connStatus = 1;  
  }else{
    Serial.print("Error in up Log, ");
    failConnect();
  }
}
//------------------End fungsi kirim data log----------------------
//------------Fungsi Read data Setting Fingger Alat----------------
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
    failConnect();
  } 
}
//----------End Fungsi Read data Setting Fingger Alat-------------

void loop() {   
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SPO2 = pox.getSpO2();          
    Serial.print("Real Time Heart rate:");    
    Serial.print("bpm / SpO2:");
    Serial.print(BPM);Serial.print("/");Serial.print(SPO2);        
    Serial.println("%"); 
    tsLastReport = millis();
  }
    
  if(Serial.available() && !onProcess){
    onProcess = true;
    String s = Serial.readStringUntil('\n');
    
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
    String data[100];
    int r = 0;
    int t = 0;
    for(int i=0;i<s.length();i++){
      if(s.charAt(i)==','){
        data[t] = s.substring(r,i);
        r = i+1;
        t++;      
      }
    }  
    // PARSE SERIAL INPUT -------------------------------------

    if(data[0]=="hasil"){      
      Serial.println("enfinger1");
      if(Firebase.RTDB.setString(&fbdo,"test/hasil", data[1])){
        Serial.println("Upload data berhasil");
      }else{
        Serial.print("Error in up data, ");
        Serial.println(fbdo.errorReason());    
      } 
    }    

    // GET SIMPLE STATUS --------------------------------------
    if(data[0]=="espStatus"){
      // 0 = NOT CONNECTED
      // 1 = CONNECTED TO AP
      // 2 = AS AP
      checkConn();
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // GET SIMPLE STATUS --------------------------------------

    // GET SET/GET GPIO STATUS --------------------------------
    if(data[0]=="espGPIO0"){
      if(data[1]=="0" || data[1]=="1" || data[1]=="LOW" || data[1]=="HIGH"){
        // SET GPIO STATUS
        pinMode(0,OUTPUT);
        digitalWrite(0,data[1].toInt());
        Serial.print("espGPIO0,");Serial.println(digitalRead(0));        
      }else{
        // GET GPIO STATUS
        pinMode(0,INPUT);
        Serial.print("espGPIO0,");Serial.println(digitalRead(0));       
      }
    }
    if(data[0]=="espGPIO2"){
      if(data[1]=="0" || data[1]=="1" || data[1]=="LOW" || data[1]=="HIGH"){
        // SET GPIO STATUS
        pinMode(2,OUTPUT);
        digitalWrite(2,data[1].toInt());
        Serial.print("espGPIO2,");Serial.println(digitalRead(2));
      }else{
        // GET GPIO STATUS
        pinMode(2,INPUT);
        Serial.print("espGPIO2,");Serial.println(digitalRead(2));        
      }
    }
    // GET SET/GET GPIO STATUS --------------------------------

    // DISCONNECT ESP -----------------------------------------
    if(data[0]=="espDisconnect"){
      disconnectESP();
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // DISCONNECT ESP -----------------------------------------

    // GET ALL STATUS -----------------------------------------
    if(data[0]=="espGetStatus"){      
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
    if(data[0]=="espHelp"){
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
    if(data[0]=="espClientMode"){
      disconnectESP();
      WiFi.begin(data[1], data[2]);    
      int i=0;
      while (WiFi.status() != WL_CONNECTED)
      {
        i++;
        delay(500);
        if(i>20){
          // break after 10s waiting
          modeESP8266 = 0;
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
    if(data[0]=="espApMode"){      
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
      if(modeESP8266==2 && WiFi.softAP(data[1], data[2],false,8)){
        modeESP8266 = 2;
        WiFi.setAutoConnect( true );
        WiFi.setAutoReconnect( true );
      }else{
        modeESP8266 = 0;
      }
      Serial.print("espMode,");Serial.println(modeESP8266);
    }
    // SET ESP8266 AS AN ACCESS POINT -------------------------

//    // HTTP REQUEST -------------------------------------------
//    if(data[0]=="espHttp"){
//      checkConn();
//      if(modeESP8266>0){
//          String url = "";
//          String param = "";
//          if(data[1]=="GET"){
//            for(int i=2;i<100;i++){
//              if(data[i]!=NULL){
//                if(url.length()>0){
//                  url = url+",";  
//                }
//                url = url+data[i];
//              }
//            }
//          }
//
//          if(data[1]=="POST"){
//            url = data[2];
//            for(int i=3;i<100;i++){
//              if(data[i]!=NULL){
//                if(param.length()>0){
//                  param = param+",";  
//                }
//                param = param+data[i];
//              }
//            }
//          }
//          //url = "http://"+url;
//          HTTPClient http;
//          
//          http.begin(url);
//          int httpCode;
//          if(data[1]=="GET"){
//            httpCode = http.GET();
//          }
//          
//          if(data[1]=="POST"){
//            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//            httpCode = http.POST(param);
//          }
//          
//          if (httpCode == HTTP_CODE_OK){
//            String payload = http.getString();
//            Serial.print("espHttpData,1,");
//            Serial.println(payload);
//          }else{
//            Serial.print("espHttpData,0,");
//            Serial.println(http.errorToString(httpCode).c_str());
//          }
//          http.end(); 
//          onRequest = false;
//      }else{
//        Serial.println("espHttpData,0,No Connection");        
//      }
//    }
//    // HTTP REQUEST -------------------------------------------
    onProcess = false;
  }  
}

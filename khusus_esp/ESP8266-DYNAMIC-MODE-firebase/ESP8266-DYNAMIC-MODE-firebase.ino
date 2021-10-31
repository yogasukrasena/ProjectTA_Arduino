#include <Firebase_ESP_Client.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool onProcess = false;
bool onRequest = false;

int modeESP8266 = 0;

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "taproject-53d6c-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBB26FHburuVXXyCf4PbmI5t18oc4j9Qt8"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "yoga@admin.com"
#define USER_PASSWORD "12Yoga12"

/* 4. Define FirebaseESP8266 data object for data sending and receiving */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig data for config data */
FirebaseConfig config;

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

void setup() {
  Serial.begin(9600);  
  Serial.println();

  timeClient.begin();
  timeClient.setTimeOffset(28800);

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
  
  disconnectESP();
}

void loop() {  
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
    // KIRIM DATA KE FIREBASE----------------------------------
    if(data[0]=="espSendData" && modeESP8266 == 1){
      
    }
            
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
    onProcess = false;
  }
}

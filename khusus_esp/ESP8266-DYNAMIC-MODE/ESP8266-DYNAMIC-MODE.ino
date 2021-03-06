#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

bool onProcess = false;
bool onRequest = false;

int modeESP8266 = 0;

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

    // HTTP REQUEST -------------------------------------------
    if(data[0]=="espHttp"){
      checkConn();
      if(modeESP8266>0){
          String url = "";
          String param = "";
          if(data[1]=="GET"){
            for(int i=2;i<100;i++){
              if(data[i]!=NULL){
                if(url.length()>0){
                  url = url+",";  
                }
                url = url+data[i];
              }
            }
          }

          if(data[1]=="POST"){
            url = data[2];
            for(int i=3;i<100;i++){
              if(data[i]!=NULL){
                if(param.length()>0){
                  param = param+",";  
                }
                param = param+data[i];
              }
            }
          }
          //url = "http://"+url;
          HTTPClient http;
          
          http.begin(url);
          int httpCode;
          if(data[1]=="GET"){
            httpCode = http.GET();
          }
          
          if(data[1]=="POST"){
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            httpCode = http.POST(param);
          }
          
          if (httpCode == HTTP_CODE_OK){
            String payload = http.getString();
            Serial.print("espHttpData,1,");
            Serial.println(payload);
          }else{
            Serial.print("espHttpData,0,");
            Serial.println(http.errorToString(httpCode).c_str());
          }
          http.end(); 
          onRequest = false;
      }else{
        Serial.println("espHttpData,0,No Connection");        
      }
    }
    // HTTP REQUEST -------------------------------------------
    onProcess = false;
  }
}

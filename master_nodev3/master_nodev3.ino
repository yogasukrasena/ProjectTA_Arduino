#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>


SoftwareSerial wifiSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino
SoftwareSerial gpsSerial(0, 2); //rx(D2), tx(D1) untuk menghubungkan dengan gps

TinyGPSPlus gps;  // The TinyGPS++ object

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
float latVar = 0;
float lngVar = 0;
// variable penerima data dari arduino
String dt[40];
String df[40];
String dataIn;
boolean parsing=false;
int i;
int finger;

WiFiServer server(80);

LiquidCrystal_I2C lcd(0x27, 16, 2);

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
  Serial.begin(9600);
  wifiSerial.begin(9600);
  gpsSerial.begin(9600);
  lcd.begin();
  while (!Serial) continue;
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //connecting to wifi
  while (WiFi.status() != WL_CONNECTED)// while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());  // Print the IP address
  Serial.println(WiFi.macAddress());  // Print the mac address

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
  idAlat();
}

void idAlat(){
  if(Firebase.RTDB.setInt(&fbdo, "Device/id", 1)&&
     Firebase.RTDB.setString(&fbdo, "Device/username", "tongkatSatu")&&
     Firebase.RTDB.setString(&fbdo, "Device/password", "tongkatSatu")&&
     Firebase.RTDB.setInt(&fbdo, "Finger/status", 0)){  
    //success
    Serial.println("Set Identias Alat success");
  }else{
    //Failed?, get the error reason from fbdo
    Serial.print("Error in setDouble, ");
    Serial.println(fbdo.errorReason());
  }
}

void dataGPS(float value1, float value2){
  //data lokasi
  Firebase.RTDB.setDouble(&fbdo, "location/lat", value1);
  Firebase.RTDB.setDouble(&fbdo, "location/lng", value2); 
    
  Serial.println("Data GPS Sukses di Upload");
}

void dataDevice(int value3, int value4, int value5, int value6, String value7){
  //data baterai dan bpm
  Firebase.RTDB.setInt(&fbdo, "battery", value3);
  Firebase.RTDB.setInt(&fbdo, "bpm", value4);
  //data waktu
  Firebase.RTDB.setInt(&fbdo, "waktu/jam", value5);
  Firebase.RTDB.setInt(&fbdo, "waktu/menit", value6);

  Firebase.RTDB.setString(&fbdo, "Finger/fingerID", value7);
  
  Serial.println("Data Device Sukses di Upload");  
}

void loop() {
//gpsSerial.setTimeout(50); 
  while(gpsSerial.available()){
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()){
//--------Read data dari firebase dan mengirimkan ke arduino----------
      if(Firebase.RTDB.get(&fbdo, "Finger/status")){
        if(fbdo.stringData().startsWith("enfinger")){          
          wifiSerial.println(fbdo.stringData());      
          Serial.println("enroll finger");
          delay(1000);
          Firebase.RTDB.setDouble(&fbdo, "Finger/status", 0);               
        }else if(fbdo.stringData().startsWith("delfinger")){          
          wifiSerial.println(fbdo.stringData());      
          Serial.println("delete finger");
          delay(1000);
          Firebase.RTDB.setDouble(&fbdo, "Finger/status", 0);               
        }else{           
          Serial.println("Tidak Krm Data ...");
        }      
      }else{
        //Failed?, get the error reason from fbdo    
        Serial.print("Error in get, ");
        Serial.println(fbdo.errorReason());
      }
//--------End Read data dari firebase dan mengirimkan ke arduino----------       
//----------------Read data parsing dari arduino-----------------------
      while(wifiSerial.available()>0) {
      //wifiSerial.setTimeout(50);
        char inChar = (char)wifiSerial.read();
        dataIn += inChar;
        if (inChar == '\n'){
          parsing = true;
        }
      }
      if(parsing){
        parsingData();
        parsing=false;
        dataIn="";       
      }
//----------------End Read data parsing dari arduino-----------------------      
//------------------------PRINT DATA GPS-----------------------------------
      lngVar = gps.location.lng(); 
      latVar = gps.location.lat();                
      Serial.print("Latitude= "); 
      Serial.print(latVar, 9);
      Serial.print(" Longitude= "); 
      Serial.println(lngVar, 9);            
//      dataGPS(latVar, lngVar);
      Serial.println("---------------------xxxxx--------------------");
//------------------------END PRINT DATA GPS-------------------------------                  
    }
  }  
}

void parsingData(){
 int j=1;
   
  //kirim data yang telah diterima sebelumnya
//  Serial.print("DATA DITERIMA : ");
//  Serial.print(dataIn);
//----------------------PARSING SELURUH DATA---------------------------   
  dt[j]="";
  for(i=1;i<dataIn.length();i++){
    if ((dataIn[i] == ':')){
      j++;
      dt[j]="";
    }
    else{
      dt[j] = dt[j] + dataIn[i];
    }
  }
//---------------------PRINT DATA YANG TELAH DI PARSING------------------
  Serial.print("Data 1 (flag) : ");
  Serial.println(dt[1]);
  Serial.print("Data 2 (battery) : ");
  Serial.println(dt[2].toInt());
  Serial.print("Data 3 (bpm) : ");
  Serial.println(dt[3].toInt());  
  Serial.print("Data 4 (button value) : ");
  Serial.println(dt[4].toInt());
  Serial.print("Data 5 id finger : ");
  Serial.println(dt[5]);
  Serial.print("Data 6 (jam) : ");
  Serial.println(dt[6].toInt());   
  Serial.print("Data 7 (menit) : ");
  Serial.println(dt[7].toInt());
  lcd.setCursor(0,0);  
  lcd.print("Device Connected");
// -------------------------KIRIM DATA KE FIREBASE-------------------------------  
  dataDevice(dt[2].toInt(),dt[3].toInt(),dt[6].toInt(),dt[7].toInt(), dt[5]);
}

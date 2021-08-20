#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>


SoftwareSerial wifiSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino
SoftwareSerial gpsSerial(4, 5); //rx(D2), tx(D1) untuk menghubungkan dengan gps

TinyGPSPlus gps;  // The TinyGPS++ object

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
float latVar = 0;
float lngVar = 0;
// variable penerima data dari arduino
int battery;
int bpm;
int push_status;
int detik;
int jam;


WiFiServer server(80);

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
}

void kirimData(float value1, float value2, int value3, int value4, int value5, int value6){
  //data lokasi
  Firebase.RTDB.setDouble(&fbdo, "location/lat", value1);
  Firebase.RTDB.setDouble(&fbdo, "location/lng", value2); 
  //data baterai dan bpm
  Firebase.RTDB.setInt(&fbdo, "battery", value3);
  Firebase.RTDB.setInt(&fbdo, "bpm", value4);
  //data waktu
  Firebase.RTDB.setInt(&fbdo, "waktu/jam", value5);
  Firebase.RTDB.setInt(&fbdo, "waktu/menit", value6);  
  
  Serial.println("Data Sukses di Upload");
}

//void lngFire(float value2){
//  if(Firebase.RTDB.setDouble(&fbdo, "location/lng", value2)){     
//    //success
//    Serial.println("Set lng data success");
//  }else{
//    //Failed?, get the error reason from fbdo
//    Serial.print("Error in setDouble, ");
//    Serial.println(fbdo.errorReason());
//  }
//}

void readJsonData(){
  //membaca data json dari arduino      
  StaticJsonBuffer<500> readData;
  JsonObject& root = readData.parseObject(wifiSerial);
  if (root == JsonObject::invalid())
  {    
    return;
  }      
  battery = root["battery"];
  bpm = root["bpm"];
//  push_status = root["push_status"];
//  menit = root["menit"];
//  jam = root["jam"];
  Serial.println("JSON received and parsed");
  Serial.print("Battery Capacity = ");            
  Serial.println(battery);
  Serial.print("Heart bpm = ");       
  Serial.println(bpm); 
//  Serial.print("button_status = ");
//  Serial.println(push_status); 
//  Serial.print("Time ON = ");
//  Serial.print(jam);
//  Serial.print(":");
//  Serial.println(menit);
}

void loop() {  
  gpsSerial.setTimeout(500);
  wifiSerial.setTimeout(500);
  while(gpsSerial.available()){
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()){
//      //read data json
//      StaticJsonBuffer<800> readData;           
//      JsonObject& root = readData.parseObject(wifiSerial);
//      if (root == JsonObject::invalid()){      
//        return;
//      }
//      battery = root["battery"];
//      bpm = root["bpm"];
//      detik = root["detik"];
//      Serial.print("Battery Capacity = ");            
//      Serial.println(battery);
//      Serial.print("Heart bpm = ");       
//      Serial.println(bpm);      
//      Serial.print("detik = ");       
//      Serial.println(detik); 
//      //send data json
//      StaticJsonBuffer<200> SendData;
//      JsonObject& kirim = SendData.createObject();
//      if(wifiSerial.available()){
//         if(Firebase.RTDB.get(&fbdo, "finger")){
//          if(fbdo.intData()== 222){
//            kirim["data"] = 222;
//            kirim.printTo(wifiSerial);      
//            Serial.println("krm data 222");     
//          }else if(fbdo.intData()== 333){
//            kirim["data"] = 333;
//            kirim.printTo(wifiSerial);      
//            Serial.println("krm data 333");
//          }else{
//            kirim["data"] = 111;      
//            Serial.println("Tidak Krm Data ...");
//          }      
//        }else{
//          //Failed?, get the error reason from fbdo    
//          Serial.print("Error in get, ");
//          Serial.println(fbdo.errorReason());
//        }
//      }
      
      //gps print    
      lngVar = gps.location.lng(); 
      latVar = gps.location.lat();                
      Serial.print("Latitude= "); 
      Serial.print(latVar, 9);
      Serial.print(" Longitude= "); 
      Serial.println(lngVar, 9);            
//      kirimData(latVar, lngVar, battery, bpm, jam, menit);
      Serial.println("---------------------xxxxx--------------------");      
      delay(500);                                 
    }    
  }
}

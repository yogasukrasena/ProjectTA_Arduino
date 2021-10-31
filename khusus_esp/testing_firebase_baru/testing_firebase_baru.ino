
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/
//
//#if defined(ESP32)
//#include <WiFi.h>
//#include <FirebaseESP32.h>
//#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
//#include <FirebaseESP8266.h>
//#endif

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "mantap_mantap"
#define WIFI_PASSWORD "shittman"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBB26FHburuVXXyCf4PbmI5t18oc4j9Qt8"

/* 3. Define the RTDB URL */
#define DATABASE_URL "taproject-53d6c-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "yoga@admin.com"
#define USER_PASSWORD "12Yoga12"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

//variable setup device
String MacAddress = WiFi.macAddress();
String nameDevice = "Device2_asdajdhjkahdkjasd";
String ipAddress;
String nameLog;

int connStatus, gpsStatus;

void setup()
{

  Serial.begin(115200);

  timeClient.begin();
  timeClient.setTimeOffset(28800);  

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
}

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
    if(Firebase.get(fbdo,nameDevice+"/user_pengguna")){
      if(fbdo.stringData()=="null"){
        Firebase.setString(fbdo,nameDevice+"/user_pengguna", "userPengguna");
      }else{
        Serial.println("Data Pengguna Tersedia");
      }
    }
    if(Firebase.get(fbdo,nameDevice+"/user_keluarga")){
      if(fbdo.stringData()=="null"){
        Firebase.setString(fbdo,nameDevice+"/user_keluarga", "userKeluarga");
      }else{
        Serial.println("Data Keluarga Tersedia");
      }
    }
    Firebase.setString(fbdo,nameDevice+"/ip_address", ipAddress);
    //set flag data    
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_device", connStatus);
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_gps", gpsStatus);
    Firebase.setDouble(fbdo,nameDevice+"/flag_status/status_fingger", 0);
    Firebase.setInt(fbdo,nameDevice+"/flag_status/status_button", 0);
    //set raw data
    Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Lat", 0);
    Firebase.setDouble(fbdo, nameDevice+"/raw_data/GPS_Long", 0);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/bpm_level", 0);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/spo2_level", 0);
    Firebase.setInt(fbdo, nameDevice+"/raw_data/battery_level", 0);
    Firebase.setString(fbdo, nameDevice+"/raw_data/fingger_data", "0");
    //set log data
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
    yield();    
  }else{
    Serial.print("Error in up Log Data, ");
    Serial.println(fbdo.errorReason());   
  }
}

void loop()
{
  setFirebase();
//    Serial.printf("Set bool... %s\n", Firebase.setBool(fbdo, "/test/bool", count % 2 == 0) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get bool... %s\n", Firebase.getBool(fbdo, "/test/bool") ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());
//
//    bool bVal;
//    Serial.printf("Get bool ref... %s\n", Firebase.getBool(fbdo, "/test/bool", &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());
//
//    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, "/test/int", count) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, "/test/int") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
//
//    int iVal = 0;
//    Serial.printf("Get int ref... %s\n", Firebase.getInt(fbdo, "/test/int", &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/test/float", count + 10.2) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get float... %s\n", Firebase.getFloat(fbdo, "/test/float") ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, "/test/double", count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, "/test/double") ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, "/test/string", "Hello World!") ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, "/test/string") ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//
//    Serial.println();
    
  //Flash string (PROGMEM and  (FPSTR), String,, String C/C++ string, const char, char array, string literal are supported
  //in all Firebase and FirebaseJson functions, unless F() macro is not supported.

//  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
//  {
//    sendDataPrevMillis = millis();
//
//    Serial.printf("Set bool... %s\n", Firebase.setBool(fbdo, "/test/bool", count % 2 == 0) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get bool... %s\n", Firebase.getBool(fbdo, "/test/bool") ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());
//
//    bool bVal;
//    Serial.printf("Get bool ref... %s\n", Firebase.getBool(fbdo, "/test/bool", &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());
//
//    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, "/test/int", count) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, "/test/int") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
//
//    int iVal = 0;
//    Serial.printf("Get int ref... %s\n", Firebase.getInt(fbdo, "/test/int", &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, "/test/float", count + 10.2) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get float... %s\n", Firebase.getFloat(fbdo, "/test/float") ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, "/test/double", count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, "/test/double") ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, "/test/string", "Hello World!") ? "ok" : fbdo.errorReason().c_str());
//
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, "/test/string") ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//
//    Serial.println();
//    
//    //For generic set/get functions.
//
//    //For generic set, use Firebase.set(fbdo, <path>, <any variable or value>)
//
//    //For generic get, use Firebase.get(fbdo, <path>).
//    //And check its type with fbdo.dataType() or fbdo.dataTypeEnum() and
//    //cast the value from it e.g. fbdo.to<int>(), fbdo.to<std::string>().
//
//    //The function, fbdo.dataType() returns types String e.g. string, boolean,
//    //int, float, double, json, array, blob, file and null.
//
//    //The function, fbdo.dataTypeEnum() returns type enum (number) e.g. fb_esp_rtdb_data_type_null (1),
//    //fb_esp_rtdb_data_type_integer, fb_esp_rtdb_data_type_float, fb_esp_rtdb_data_type_double,
//    //fb_esp_rtdb_data_type_boolean, fb_esp_rtdb_data_type_string, fb_esp_rtdb_data_type_json,
//    //fb_esp_rtdb_data_type_array, fb_esp_rtdb_data_type_blob, and fb_esp_rtdb_data_type_file (10)
//
//    count++;
//  }
}

/// PLEASE AVOID THIS ////

//Please avoid the following inappropriate and inefficient use cases
/**
 * 
 * 1. Call get repeatedly inside the loop without the appropriate timing for execution provided e.g. millis() or conditional checking,
 * where delay should be avoided.
 * 
 * Every time get was called, the request header need to be sent to server which its size depends on the authentication method used, 
 * and costs your data usage.
 * 
 * Please use stream function instead for this use case.
 * 
 * 2. Using the single FirebaseData object to call different type functions as above example without the appropriate 
 * timing for execution provided in the loop i.e., repeatedly switching call between get and set functions.
 * 
 * In addition to costs the data usage, the delay will be involved as the session needs to be closed and opened too often
 * due to the HTTP method (GET, PUT, POST, PATCH and DELETE) was changed in the incoming request. 
 * 
 * 
 * Please reduce the use of swithing calls by store the multiple values to the JSON object and store it once on the database.
 * 
 * Or calling continuously "set" or "setAsync" functions without "get" called in between, and calling get continuously without set 
 * called in between.
 * 
 * If you needed to call arbitrary "get" and "set" based on condition or event, use another FirebaseData object to avoid the session 
 * closing and reopening.
 * 
 * 3. Use of delay or hidden delay or blocking operation to wait for hardware ready in the third party sensor libraries, together with stream functions e.g. Firebase.readStream and fbdo.streamAvailable in the loop.
 * 
 * 
 * Please use non-blocking mode of sensor libraries (if available) or use millis instead of delay in your code.
 * 
 */

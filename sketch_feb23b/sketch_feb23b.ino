#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

SoftwareSerial wifiSerial(12, 14); //rx, tx
TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial gpsSerial(4, 5); //rx, tx
const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
float latitude , longitude;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;
int pm;
WiFiServer server(80);

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
 
}
 
void loop() {
  while (gpsSerial.available() > 0){
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()){      
      StaticJsonBuffer<1000> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(wifiSerial);
      if (root == JsonObject::invalid())
        return;
      Serial.println("JSON received and parsed");    
      Serial.print("Data 1 ");      
      int data1=root["data1"];
      Serial.println(data1);
      Serial.print("Data 2 ");
      int data2=root["data2"];  
      Serial.println(data2);
      Serial.print("Data ultra ");
      int data3=root["ultra"];
      Serial.println(data3);      
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
      Serial.println("---------------------xxxxx--------------------");
      delay(500);         
    }    
  } 
}

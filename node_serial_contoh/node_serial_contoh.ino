#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

SoftwareSerial wifiSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
int battery;
int bpm;

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  // Initialize Serial port
  Serial.begin(9600);
  wifiSerial.begin(9600);  
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
  //kirim data json ke arduino  
  StaticJsonBuffer<200> SendData;
  StaticJsonBuffer<500> readData;
  JsonObject& kirim = SendData.createObject();
    kirim["data"] = 222;
  if(wifiSerial.available()){
    wifiSerial.setTimeout(500);    
    kirim.printTo(wifiSerial);
    JsonObject& root = readData.parseObject(wifiSerial);
    if(root == JsonObject::invalid()){      
      return;      
    }
    battery = root["battery"];
    bpm = root["bpm"];
    Serial.println("JSON received and parsed");
    Serial.print("Battery Capacity = ");            
    Serial.println(battery);
    Serial.print("Heart bpm = ");       
    Serial.println(bpm);     
    delay(500);        
  }
  delay(500); 
}

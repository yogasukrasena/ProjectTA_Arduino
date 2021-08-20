#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

SoftwareSerial wifiSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
String dt[40];
String dataIn;
boolean parsing=false;
int i;

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
  wifiSerial.println("222");
  Serial.println("Data Terkirim ...");
  delay(500);  
  while(wifiSerial.available()>0) {
    wifiSerial.setTimeout(500);    
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
}


void parsingData(){
 int j=1;
   
  //kirim data yang telah diterima sebelumnya
  Serial.print("DATA DITERIMA : ");
  Serial.print(dataIn);
   
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
  Serial.print("Data 1 (flag) : ");
  Serial.println(dt[1].toInt());
  Serial.print("Data 2 (menit) : ");
  Serial.println(dt[2].toInt());
  Serial.print("Data 3 (detik) : ");
  Serial.println(dt[3].toInt());  
}

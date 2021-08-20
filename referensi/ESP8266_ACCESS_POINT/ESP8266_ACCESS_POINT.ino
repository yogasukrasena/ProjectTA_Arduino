#include <SoftwareSerial.h>
SoftwareSerial espSerial(9, 8); // TX, RX

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  espSerial.begin(9600);

  // request help
  espSerial.println("espHelp");
  while(!espSerial.available()){
  } 
  // menampilkan hasil request help
  String s = espSerial.readString();
  Serial.println(s);
  
  //test ESP8266 sebagai Akses Point
  espSerial.println("espApMode,test-ssid,password");
  while(!espSerial.available()){
  } 

  // manmpilkan status sebagai AP
  s = espSerial.readString();
  Serial.println(s);
}

void loop() {
  // put your main code here, to run repeatedly:

}

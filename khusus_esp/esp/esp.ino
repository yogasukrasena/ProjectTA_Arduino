#include <SoftwareSerial.h>
SoftwareSerial espSerial(6, 7); // TX, RX

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  //
  Serial.print("Trying connect to AP ... ");
  espSerial.println("espClientMode,Tewe,1234567890");
  while(!espSerial.available()){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected !");
}

void loop() {
  //Request ke laman web
  espSerial.println("espHttp,GET,http://sekalahita.id");

  // Menampilkan respon dari pemanggilan halaman web
  while(espSerial.available()){
    String s = espSerial.readStringUntil('\n');
    Serial.println(s);
  } 
  delay(1000);     
}

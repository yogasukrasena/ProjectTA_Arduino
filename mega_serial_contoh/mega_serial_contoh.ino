#include <ArduinoJson.h>
//serial tx(16) dan rx (17) untuk nodemcu
#define wifiSerial Serial1

#define echoFront 38 //echo pin
#define trigFront 39 //trig pin

int data;
String str[40];
String data1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  wifiSerial.begin(9600);

  Serial.println("Serial Test");
  // initialize the ultrasonic
  pinMode(trigFront, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoFront, INPUT); // Sets the echoPin as an INPUT 
}

int UltraFront(){
  
  long durationFront; 
  int distanceFront;  

  // Clears the trigPin condition
  digitalWrite(trigFront, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigFront, HIGH); delayMicroseconds(10);
  digitalWrite(trigFront, LOW);    
  durationFront = pulseIn(echoFront, HIGH);
  // Calculating the distance
  distanceFront = durationFront * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)  
  return distanceFront;
}

void loop() {
  StaticJsonBuffer<1000> jsonBuffer;
  StaticJsonBuffer<200> jsonRecive;
  JsonObject& root = jsonBuffer.createObject();   
    root["battery"] = 100;
    root["bpm"] = 80;
  if(wifiSerial.available()){
     wifiSerial.setTimeout(500);    
     root.printTo(wifiSerial);        
     JsonObject& recive = jsonRecive.parseObject(wifiSerial);
     if (recive == JsonObject::invalid()){                
        return;        
     }
      Serial.print("Data yg diterima : ");
      int dataRev = recive["data"];
      Serial.println(dataRev);
      delay(500);            
   }
   delay(500);
}

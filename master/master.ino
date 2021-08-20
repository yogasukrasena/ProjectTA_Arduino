#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#define s Serial3

#define echoFront 22 //echo pin
#define trigFront 23 //trig pin

void setup() {
  pinMode(trigFront, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoFront, INPUT); // Sets the echoPin as an INPUT  
  Serial.begin(9600);
  s.begin(9600);  
}
 
void loop() {  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["data1"] = 100;
  root["data2"] = 200;
  root["ultra"] = UltraFront();
  if(s.available()>0){
     root.printTo(s);
     int data1 = root["data1"];
     int data2 = root["data2"];   
     int ultra = root["ultra"];
     Serial.println(data1);
     Serial.println(data2);
     Serial.println(UltraFront());
     delay(500);
  }    
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

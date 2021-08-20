/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>

// The TinyGPS++ object
TinyGPSPlus gps;

//serial tx(16) dan rx (17) for finggerprint
#define wifiSerial Serial3

// The serial connection to the GPS device
#define fpSerial Serial2

//ultrasonic define pin
#define echoFront 38 //echo pin
#define trigFront 39 //trig pin
#define echoRight 22 
#define trigRight 23 
#define echoLeft 52 
#define trigLeft 53 

const int soilPin = A0; 
const int buttonPin = 2;     // the number of the pushbutton pin
const int buzzer = 9; //buzzer to arduino pin 9
const int ledPin =  13;      // the number of the LED pin
const int vibFront = 3;
const int vibRight = 4;
const int vibLeft = 5;

// variables will change:
int buttonState = 0;
int soilValue;
int finggerID = 0;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);

void setup()
{
  Serial.begin(9600);
  wifiSerial.begin(9600);  
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // initialize the pushbutton pin and buzzer
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer, OUTPUT);  
  pinMode(buttonPin, INPUT);

  // intialize the vibrate
  pinMode(vibFront, OUTPUT);
  pinMode(vibRight, OUTPUT);
  pinMode(vibLeft, OUTPUT);

  // initialize the ultrasonic
  pinMode(trigFront, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoFront, INPUT); // Sets the echoPin as an INPUT  
  pinMode(trigRight, OUTPUT); 
  pinMode(echoRight, INPUT); 
  pinMode(trigLeft, OUTPUT); 
  pinMode(echoLeft, INPUT); 

  //intitialize finggerprint
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() 
{
  finggerID = getFingerprintIDez();
  while(finggerID == 1){        
    StaticJsonBuffer<1000> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
      root["data1"] = 100;
      root["data2"] = 200;
      root["ultra"] = UltraFront();
    if(wifiSerial.available()>0){
       root.printTo(wifiSerial);
       int data1 = root["data1"];
       int data2 = root["data2"];   
       int ultra = root["ultra"];
       Serial.println(data1);
       Serial.println(data2);
       Serial.println(UltraFront());       
    }        
      Button();
      Serial.print("Fingger ID :");
      Serial.println(finggerID);
      //print ultrasonic
      Serial.print("Distance center: ");
      Serial.print(UltraFront());
      Serial.println(" cm"); 
      Serial.print("Distance right: ");
      Serial.print(UltraRight());
      Serial.println(" cm"); 
      Serial.print("Distance left: ");
      Serial.print(UltraLeft());
      Serial.println(" cm");
      //print soil
      Serial.print("nilai soil :");
      Serial.println(soilValue = analogRead(soilPin));          
      delay(500); 
   } 
   Serial.println("Scan Your Fingger");
   Serial.print("Fingger ID :");
   Serial.println(finggerID);
   delay(500);
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  tone(buzzer, 1000);
  delay(1000);
  noTone(buzzer);
  return finger.fingerID;  
}

int Button() {
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    Serial.print("Push Button : ");
    Serial.println(buttonState);                    
    return buttonState;
  } else {
    // turn LED off:
    Serial.print("unpush button : ");
    Serial.println(buttonState);    
    return buttonState;     
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
  if(distanceFront < 100){
    digitalWrite(vibFront, HIGH); //vibrate
    delay(500);  // delay one second
    digitalWrite(vibFront, LOW);  //stop vibrating
    delay(500); //wait 50 seconds. 
  }    
  return distanceFront;
}

int UltraRight(){
  
  long durationRight; 
  int distanceRight;  

  // Clears the trigPin condition
  digitalWrite(trigRight, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigRight, HIGH); delayMicroseconds(10);
  digitalWrite(trigRight, LOW);    
  durationRight = pulseIn(echoRight, HIGH);
  // Calculating the distance
  distanceRight = durationRight * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  if(distanceRight < 100){
    digitalWrite(vibRight, HIGH); //vibrate
    delay(500);  // delay one second
    digitalWrite(vibRight, LOW);  //stop vibrating
    delay(500); //wait 50 seconds. 
  }    
  return distanceRight;
}

int UltraLeft(){
  
  long durationLeft; 
  int distanceLeft;  

  // Clears the trigPin condition
  digitalWrite(trigLeft, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigLeft, HIGH); delayMicroseconds(10);
  digitalWrite(trigLeft, LOW);    
  durationLeft = pulseIn(echoLeft, HIGH);
  // Calculating the distance
  distanceLeft = durationLeft * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  if(distanceLeft < 100){
    digitalWrite(vibLeft, HIGH); //vibrate
    delay(500);  // delay one second
    digitalWrite(vibLeft, LOW);  //stop vibrating
    delay(500); //wait 50 seconds. 
  }    
  return distanceLeft;
}

int soilNilai(){

  int soilValue;  
  int limit = 300; 

  soilValue = analogRead(soilPin); 
  Serial.println("Analog Value : ");
  Serial.println(soilValue);

  return soilValue;
}

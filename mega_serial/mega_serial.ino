#include <ArduinoJson.h>
//serial tx(16) dan rx (17) untuk nodemcu
#define wifiSerial Serial1

#define echoFront 38 //echo pin
#define trigFront 39 //trig pin

int data;
String dt[40];
String dataIn;
boolean parsing = false;

//variabel timer on alat
unsigned long milisec;
unsigned long detik;
unsigned long menit;
unsigned long jam;

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

void timerAlat(){
  milisec = millis();
  detik = milisec/1000;
  menit = detik/60;
  jam   = menit/60;

  milisec %= 1000;
  detik %= 60;
  menit %= 60;
  jam %= 24;

//  Serial.print("Timer ON : ");  
//  Serial.print(jam);
//  Serial.print(": ");
//  Serial.print(menit);
//  Serial.print(": ");
//  Serial.println(detik);
  
}

void loop() {
  timerAlat();
  wifiSerial.print("10");
  wifiSerial.print(":");
  wifiSerial.print(menit);
  wifiSerial.print(":");
  wifiSerial.println(detik);
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
//  dt[j]="";
//  for(i=1;i<dataIn.length();i++){
//    if ((dataIn[i] == ':')){
//      j++;
//      dt[j]="";
//    }
//    else{
//      dt[j] = dt[j] + dataIn[i];
//    }
//  }
  
}

// ---------------------------------------------------------------- //
// Arduino Ultrasoninc Sensor HC-SR04
// Re-writed by Arbi Abdul Jabbaar
// Using Arduino IDE 1.8.7
// Using HC-SR04 Module
// Tested on 17 September 2019
// ---------------------------------------------------------------- //

#define echoFront 22 //echo pin
#define trigFront 23 //trig pin
#define echoRight 24 
#define trigRight 25 
#define echoLeft 26 
#define trigLeft 27 

void setup() {
  pinMode(trigFront, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoFront, INPUT); // Sets the echoPin as an INPUT  
  pinMode(trigRight, OUTPUT); 
  pinMode(echoRight, INPUT); 
  pinMode(trigLeft, OUTPUT); 
  pinMode(echoLeft, INPUT); 
  Serial.begin(9600);
  Serial.println("Ultrasonic Sensor HC-SR04 Test");
  Serial.println("with Arduino MEGA");
}
void loop() {  
  
  // Displays the distance on the Serial Monitor
  Serial.print("Distance center: ");
  Serial.print(UltraFront());
  Serial.println(" cm"); 
  Serial.print("Distance right: ");
  Serial.print(UltraRight());
  Serial.println(" cm"); 
  Serial.print("Distance left: ");
  Serial.print(UltraLeft());
  Serial.println(" cm"); 
  delay(500);
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
  return distanceLeft;
}

// Include NewPing Library
#include <NewPing.h>

// Hook up HC-SR04 with Trig to Arduino Pin 9, Echo to Arduino pin 10
#define TRIGGER_PIN A3
#define ECHO_PIN A4

// Maximum distance we want to ping for (in centimeters).
#define MAX_DISTANCE 400  

// NewPing setup of pins and maximum distance.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
int duration, distance;

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  // Send ping, get distance in cm
  distance = sonar.ping_cm();  

  float hasil = (distance + 0.6805) / 0.9967;
  float hasil1 = distance;
  
  // Send results to Serial Monitor
  Serial.print("Distance = ");
  Serial.print(hasil1);
  Serial.println(" cm");
  
  delay(1000);
}

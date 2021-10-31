int offset =20;// set the correction offset value

float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;

#define vibrate 2

void setup() {
  // Robojax.com voltage sensor
  Serial.begin(9600);

  pinMode(vibrate, OUTPUT);
}

void loop() {
  int volt = analogRead(A0);// read the input 
  vOUT = (volt * 5.0) / 1024.0;
  vIN = vOUT / (R2/(R1+R2));
  Serial.print("Input = ");
  Serial.println(vIN);
  digitalWrite(vibrate, HIGH);  
  delay(500);
  digitalWrite(vibrate, LOW);  
}

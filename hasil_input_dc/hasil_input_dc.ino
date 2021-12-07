#define voltIn A0

float VOut = 0.0, VIn = 0.0;
float R1 = 30000.0, R2 = 7500.0;
float Vmin = 3.15, Vmax = 4.25;
int battery = 0;
float hasil = 0;

void setup() {
  // Robojax.com voltage sensor  
  Serial.begin(9600);
//  analogReference(INTERNAL1V1); 
  analogReference(INTERNAL2V56);
}

void loop() {
//  int volt = analogRead(A0)-26;// read the input 
  long t1 = millis();
  int volt = analogRead(voltIn);
  VOut = (volt * 2.56)/1024;
  VIn = VOut / (R2/(R1+R2));
  Serial.print("Input before = ");  
  Serial.println(VIn);  
  
  hasil = ((VIn - Vmin) / (Vmax - Vmin)) * 100;
  battery = round(hasil);
  Serial.print("hasil volt : ");
  Serial.println(VIn - 0.92);  
//  Serial.print("sisa baterai : ");
//  if(battery < 100 && battery > 0){ 
//    Serial.println(battery);  
//  }else if (VIn < Vmin){
//    Serial.println("0"); 
//  }else{
//    Serial.println("100");
//  }
  delay(1000);
  long t2 = millis();
//  Serial.print("waktu berjalan : ");Serial.println(t2-t1);
}

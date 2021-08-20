#include <MAX30100_PulseOximeter.h>
#include <SoftwareSerial.h>
 
#define REPORTING_PERIOD_MS     1000
SoftwareSerial pulseSerial(14, 15); //rx(D7), tx(D8) untuk menghubungkan dengan pulse
 
PulseOximeter pox;
uint32_t tsLastReport = 0;
long startMillis, currentMillis;
const long period = 20000;
int BPM, SPO2;
int maxBpm, minBpm;
int maxSpo, minSpo;
int count;
 
void onBeatDetected()
{
    Serial.println("Beat!");    
}
 
void setup()
{
  Serial.begin(9200);
  pulseSerial.begin(115200);
  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }     
}
 
void loop()
{  
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SPO2 = pox.getSpO2();      
    Serial.print("Real Time Heart rate:");
    Serial.print(BPM);
    Serial.print("bpm / SpO2:");
    Serial.print(SPO2);
    Serial.println("%");    
    tsLastReport = millis();     
    if(pox.getHeartRate() > 0 && pox.getSpO2() > 0){      
      count++;      
      if(count > 10){        
        rangeBPM(BPM);
        rangeSPO2(SPO2);
      }      
    }else{
      count = 0;
    }
    Serial.print("Max Bpm :");
    Serial.print(maxBpm);
    Serial.print(" Min Bpm :");
    Serial.println(minBpm);

    Serial.print("Max Spo2 :");
    Serial.print(maxSpo);
    Serial.print(" Min Spo2 :");
    Serial.println(minSpo);

    pulseSerial.print(0xAA); pulseSerial.print(":");
    pulseSerial.print(BPM); pulseSerial.print(":");
    pulseSerial.print(SPO2); pulseSerial.print(":");
    pulseSerial.print(maxBpm); pulseSerial.print(":");
    pulseSerial.print(minBpm); pulseSerial.print(":");
    pulseSerial.print(maxSpo); pulseSerial.print(":");
    pulseSerial.println(minSpo);
    
  }
}

void rangeBPM(int data){
  if(maxBpm == NULL && minBpm == NULL){
    maxBpm = data;
    minBpm = data;
  }else if(data > maxBpm){
    maxBpm = data;
  }else if(data < minBpm){
    minBpm = data;
  }
}

void rangeSPO2(int data){
  if(maxSpo == NULL && minSpo == NULL){
    maxSpo = data;
    minSpo = data;
  }else if(data > maxSpo){
    maxSpo = data;
  }else if(data < minSpo){
    minSpo = data;
  }
}

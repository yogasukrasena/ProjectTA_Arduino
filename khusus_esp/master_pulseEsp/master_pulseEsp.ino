#include <MAX30100_PulseOximeter.h>
#include <Wire.h>
 
#define REPORTING_PERIOD_MS     1000
 
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
  Wire.begin(2,0);
  Serial.begin(9200);           
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
  pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);
}
 
void loop(){  
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SPO2 = pox.getSpO2();                       
    if(BPM > 0 && SPO2 > 0){      
      count++;      
      if(count > 10){        
        rangeBPM(BPM);
        rangeSPO2(SPO2);
      }      
    }else{
      count = 0;
    }    
    String dataHasil = String(BPM)+","+String(SPO2)+","+String(maxBpm)+","+String(minBpm)+","+String(maxSpo)+","+String(minSpo);
    Serial.println(dataHasil);

    tsLastReport = millis();     
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

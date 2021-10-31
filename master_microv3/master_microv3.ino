#include <MAX30100_PulseOximeter.h>
#include <SoftwareSerial.h>
 
#define REPORTING_PERIOD_MS     1000
SoftwareSerial pulseSerial(14, 15); //RX,TX
PulseOximeter pox;
uint32_t tsLastReport = 0;
long startMillis, currentMillis;
const long period = 20000;
int BPM, SPO2;
int maxBpm, minBpm;
int maxSpo, minSpo;
int count;

// variable penerima data dari micro
String dt2[40];
String dataIn2;
boolean parsing2 = false;
int l;
 
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
    
//    pulseSerial.println(maxBpm+","+maxSpo);       
  }

   pulseSerial.println("hasil");
    
//    while(pulseSerial.available()>0){      
//      char s = (char)pulseSerial.read();
//      dataIn2 += s;
//      if(s == '\n'){
//        Serial.println();
//        dataIn2 = ""; 
//      }      
//    }
//  while(pulseSerial.available()>0){
//    char inChar2 = (char)pulseSerial.read();
//    dataIn2 += inChar2;
//    if (inChar2 == '\n'){
//      parsing2 = true;
//    }
//  }   
//  if(parsing2){
//    parsingData2();    
//    parsing2=false;
//    dataIn2="";       
//  }
}

void parsingData2(){
 int k=1;
  Serial.println(dataIn2);    
 //--PARSING SELURUH DATA--
  dt2[k]="";
  for(l=1;l<dataIn2.length();l++){
    if ((dataIn2[l] == ',')){
      k++;
      dt2[k]="";
    }
    else{
      dt2[k] = dt2[k] + dataIn2[l];
    }
  }
  //--PRINT DATA YANG TELAH DI PARSING--
  if(dataIn2 == "hasil,hasil"){
    Serial.println("mantapp");
  }
  Serial.println("--------------Parsing Data----------------");
  Serial.print("Data 1 (flag) : ");
  Serial.println(String(dt2[1]));
  Serial.print("Data 2 (BPM) : ");
  Serial.println(String(dt2[2]));
//  Serial.print("Data 3 (SPO2) : ");
//  Serial.println(dt2[3].toInt());  
//  Serial.print("Data 4 (maxBpm) : ");
//  Serial.println(dt2[4].toInt());
//  Serial.print("Data 5 (minBpm) : ");
//  Serial.println(dt2[5].toInt());
//  Serial.print("Data 6 (maxSpo) : ");
//  Serial.println(dt2[6].toInt());   
//  Serial.print("Data 7 (minSpo) : ");
//  Serial.println(dt2[7].toInt());    
  //--KIRIM DATA KE FIREBASE--
}
//----------------------End Fungsi PARSING DATA---------------------------  

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

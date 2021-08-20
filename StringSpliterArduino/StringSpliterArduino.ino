#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>
#include <StringSplitter.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

SoftwareSerial nodemcu(2, 3); //d6, d5
SoftwareSerial portTwo(10, 11); //11,10

#define REPORTING_PERIOD_MS     1000
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

uint32_t tsLastReport = 0;
String cdata;
String tempC;

int bpm;
int spo2;
float suhu;
// tambahan
const int buttonTemp = 4;
int buttonState = 0;
int buttonStateTemp = 0;
float capture_temp;

String incomingByte;
String data_sys, data_dia;
String buff[2];

int sistole = 0;
int diastole = 0;

uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};

byte plus[] = {
  0x00,
  0x04,
  0x04,
  0x1F,
  0x04,
  0x04,
  0x00,
  0x00
};

void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
 
  Serial.begin(115200);
  nodemcu.begin(9600);
  portTwo.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  pinMode(buttonTemp, INPUT_PULLUP);
  pinMode(6, OUTPUT);
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    mlx.begin();
    
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  digitalWrite(6, HIGH);
  pox.update();
  buttonStateTemp == digitalRead(buttonTemp);
  if (digitalRead(4) == 0){
    delay(200);
    capture_temp = mlx.readObjectTempC() + 2.5;
    Serial.print("SUHU===> ");
    Serial.println(capture_temp);
    return;
  }
  suhu = mlx.readObjectTempC() + 2.5;

  portTwo.listen();
  if(portTwo.available()){
    incomingByte = portTwo.readStringUntil('\n');
    StringSplitter *splitter = new StringSplitter(incomingByte, ',', 2);
    int itemCount = splitter->getItemCount();
    for (int i = 0; i < itemCount; i++) {
      buff[i] = splitter->getItemAtIndex(i);
    }
    data_sys = buff[0];
    data_dia = buff[1];
    Serial.print("mmhg : ");
    Serial.print(data_sys);
    Serial.print("/");
    Serial.println(data_dia);
  }
  

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    measure_oxy();
    measure_temp();
    print_data();
    nodemcu.print(bpm);
    nodemcu.print(',');
    nodemcu.print(spo2);
    nodemcu.print(',');
    if(capture_temp > 0){
      nodemcu.print(capture_temp);
      nodemcu.print(',');
    } else {
      nodemcu.print(suhu);
      nodemcu.print(',');
    }
    if((data_sys == 0) && (data_dia)){
      nodemcu.print(sistole);
      nodemcu.print(',');
      nodemcu.print(diastole);
    }else{
      nodemcu.print(data_sys);
      nodemcu.print(',');
      nodemcu.print(data_dia);
    }
    
    nodemcu.print("\n");
    tsLastReport = millis();
  }
}
void print_data(){
  lcd.clear();
  lcd.setCursor(0,0);
  if(capture_temp > 0){
    lcd.print("Suhu : ");
    lcd.print(capture_temp);
  } else{
    lcd.print("Suhu : ");
    lcd.print(suhu);
  }
  lcd.setCursor(0,1);
  lcd.print("BPM  : ");
  lcd.print(bpm);
  lcd.createChar(0, heart);
  lcd.setCursor(0,2);
  lcd.print("SpO2 : ");
  lcd.print(spo2);
  lcd.setCursor(0,3);
  lcd.print("mmHg : ");
  if((data_sys == 0) && (data_dia == 0)){
    lcd.print(sistole);
    lcd.print("/");
    lcd.print(diastole);
  } else {
    lcd.print(data_sys);
    lcd.print("/");
    lcd.print(data_dia);
  }
}
void measure_temp() {
  
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC() + 2.5); Serial.println("*C");
}

void measure_oxy() {
  
  bpm = pox.getHeartRate();
  spo2 = pox.getSpO2();
  Serial.print("Heart rate:");
  Serial.print(bpm);
  Serial.print("bpm / SpO2:");
  Serial.print(spo2);
  Serial.println("%");
}

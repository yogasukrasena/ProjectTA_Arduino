#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mySerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;

void setup () {
Serial.begin (9600);
mySerial.begin (9600);

if (!myDFPlayer.begin(mySerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms  

  myDFPlayer.enableDAC();  //Enable On-chip DAC
  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).

  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  myDFPlayer.play(1);  //Play the first mp3
  delay(1000);

}

void loop () {

}

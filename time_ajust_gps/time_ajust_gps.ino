#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 0, TXPin = 2;
static const uint32_t GPSBaud = 4800;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

char wjam[10], wtanggal[10];

const int UTC_offset = 8;
time_t prevDisplay = 0;

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      time_ajust();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void time_ajust(){
  
  int Year = gps.date.year();
  byte Month = gps.date.month();
  byte Day = gps.date.day();
  byte Hour = gps.time.hour();
  byte Minute = gps.time.minute();
  byte Second = gps.time.second();

  // Set Time from GPS data string
  setTime(Hour, Minute, Second, Day, Month, Year);
  // Calc current Time Zone time by offset value
  adjustTime(UTC_offset * SECS_PER_HOUR);
         
  // -- Delete this section if not displaying time ------- //
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {
      prevDisplay = now();
      tampilkan();
    }
  }
}

void tampilkan() {
  Serial.print("Location: "); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else {
    Serial.print("Lokasi Belum Terbaca");
  }

  if (gps.date.isValid() && gps.time.isValid())
  {
    sprintf (wjam, "%02d:%02d:%02d", hour(), minute(), second());
    Serial.print("\tWaktu = ");
    Serial.print(wjam);
    sprintf (wtanggal, "%02d/%02d/%02d", day(), month(), year());
    Serial.print("Tanggal :");
    Serial.println(wtanggal);
  } else {
    Serial.println("\tWaktu Belum Terbaca");
  }

}

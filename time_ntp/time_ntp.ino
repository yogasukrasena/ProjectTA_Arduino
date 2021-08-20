#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

const char *ssid     = "Home";
const char *password = "munduksari10";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

////Week Days
//String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
////Month names
//String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup(){
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
  timeClient.setTimeOffset(28800);
}

void loop() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();

  Serial.println(timeClient.getFormattedTime());
  
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
 
  int monthDay = ptm->tm_mday;  
  int currentMonth = ptm->tm_mon+1; 
//  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900; 
  
  //Print complete date:
  String currentDate = String(monthDay)+ "/" +String(currentMonth)+ "/" + String(currentYear);
  Serial.println(currentDate);

  delay(1000);
}

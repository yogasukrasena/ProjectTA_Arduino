#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

SoftwareSerial wifiSerial(12, 14); //rx(D6), tx(D5) untuk menghubungkan dengan arduino
SoftwareSerial gpsSerial(0, 2); //rx(D2), tx(D1) untuk menghubungkan dengan gps
SoftwareSerial pulseSerial(13, 15); //rx(D7), tx(D8) untuk menghubungkan dengan pulse

TinyGPSPlus gps;  // The TinyGPS++ object

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* ssid = "mantap_mantap"; //ssid of your wifi
const char* password = "shittman"; //password of your wifi
float latVar = 0;
float lngVar = 0;

// variable penerima data dari arduino
String dt[40];
String dataIn;
boolean parsing = false;
int i, finger, connStatus, gpsStatus;

// variable penerima data dari micro
String dt2[40];
String dataIn2;
boolean parsing2 = false;
int l;

//variable ajust time rtc gps
char wjam[10], wtanggal[10];
const int UTC_offset = 8;
time_t prevDisplay = 0;

//variable setup device
char MacAddress[40];
char nameDevice[40] = "Device_";

LiquidCrystal_I2C lcd(0x27, 16, 2);

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "taproject-53d6c-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBB26FHburuVXXyCf4PbmI5t18oc4j9Qt8"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "yoga@admin.com"
#define USER_PASSWORD "12Yoga12"

#define buzzer 4
/* 4. Define FirebaseESP8266 data object for data sending and receiving */
FirebaseData fbdo;

/* 5. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 6. Define the FirebaseConfig data for config data */
FirebaseConfig config;

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  wifiSerial.begin(9600);
  gpsSerial.begin(9600);
  pulseSerial.begin(115200);
  lcd.begin();
  while (!Serial) continue;
  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //connecting to wifi
  while (WiFi.status() != WL_CONNECTED)// while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."
    connStatus = 0;    
    deviceStatus();
  }
  Serial.println("");
  Serial.println("WiFi connected");
//--convert mac address from string to char--
  String dataMac = WiFi.macAddress();
  dataMac.toCharArray(MacAddress, 40);  
  strcat(nameDevice,MacAddress); //combine hasil konversi kedalam variabel
//--End convert mac address from string to char--
  Serial.println(nameDevice);
  Serial.println(WiFi.localIP());  // Print the IP address
  Serial.println(MacAddress);  // Print the mac address  
  connStatus = 1;

  /* 7. Assign the project host and api key (required) */
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;

  /* 8. Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  /* 9. Initialize the library with the autentication data */
  Firebase.begin(&config, &auth);

  /* 10. Enable auto reconnect the WiFi when connection lost */
  Firebase.reconnectWiFi(true);
  idAlat();
}

//---------------Fungsi cek Setup Koneksi--------------------
void failConnect(){
//Failed?, get the error reason from fbdo      
  Serial.println(fbdo.errorReason());
  if (fbdo.errorReason() == "connection lost"){
    ESP.reset();
  }else if(fbdo.errorReason() == "not connected"){
    connStatus = 2;
  }
}
//---------------End Fungsi cek Setup Koneksi--------------------
//---------------Fungsi Upload Setup data Alat--------------------
void idAlat(){
  char name_id[40], user_name[40], user_keluarga[40];
  strcpy(name_id, nameDevice);
  strcpy(user_name, nameDevice);
  strcpy(user_keluarga, nameDevice);  
  if(Firebase.RTDB.setString(&fbdo,strcat(name_id,"/device_id"), MacAddress)&&
     Firebase.RTDB.setString(&fbdo,strcat(user_name,"/user_pengguna"), "userPengguna")&&
     Firebase.RTDB.setString(&fbdo,strcat(user_keluarga,"/user_keluarga"), "userKeluarga")){  
    //success
      Serial.println("Set Identias Alat success");
      connStatus = 1;
  }else{
    Serial.print("Error in set idAlat, ");
    failConnect();
  }
}
//---------------End Fungsi Upload Setup data Alat--------------------
//---------------Fungsi Upload Setup data Flag------------------------
void flagStatus(int buttonStatus){
  char status_device[40], status_gps[40], status_fingger[40], status_button[40];
  strcpy(status_device,nameDevice);
  strcpy(status_gps,nameDevice);
  strcpy(status_fingger,nameDevice);
  strcpy(status_button,nameDevice);  
  if(Firebase.RTDB.setInt(&fbdo,strcat(status_device,"/flag_status/status_device"), connStatus)&&
     Firebase.RTDB.setInt(&fbdo,strcat(status_gps,"/flag_status/status_gps"), gpsStatus)&&
     Firebase.RTDB.setDouble(&fbdo,strcat(status_fingger,"/flag_status/status_fingger"), 0)&&
     Firebase.RTDB.setInt(&fbdo,strcat(status_button,"/flag_status/status_button"), buttonStatus)){
      //success
      Serial.println("Data Flag Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in set flag, ");
    failConnect();
  }
}
//---------------End Fungsi Upload Setup data Flag---------
//---------------Fungsi Upload data GPS--------------------
void dataGPS(float gpsLat, float gpsLng){
  char lat_value[30], long_value[30];
  strcpy(lat_value,nameDevice);
  strcpy(long_value,nameDevice);  
  //data lokasi
  if(Firebase.RTDB.setDouble(&fbdo, strcat(lat_value,"/raw_data/GPS_Lat"), gpsLat)&&
     Firebase.RTDB.setDouble(&fbdo, strcat(long_value,"/raw_data/GPS_Long"), gpsLng)){
      Serial.println("Data GPS Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in up GPS, ");
    failConnect();
  }
}
//---------------End Fungsi Upload data GPS------------------
//---------------Fungsi Upload data Pulse--------------------
void dataPulse(int bpm, int spo2){
  char bpm_value[30], spo2_value[30];
  strcpy(bpm_value,nameDevice);
  strcpy(spo2_value,nameDevice); 
  //data lokasi
  if(Firebase.RTDB.setInt(&fbdo, strcat(bpm_value,"/raw_data/bpm_level"), bpm)&&
     Firebase.RTDB.setInt(&fbdo, strcat(spo2_value,"/raw_data/spo2_level"), spo2)){           
      Serial.print("Data pulse Sukses di Upload");
      Serial.println(bpm);
      connStatus = 1;
  }else{
    Serial.print("Error in up GPS, ");
    failConnect();
  }
}
//---------------End Fungsi Upload data Pulse------------------
//---------------Fungsi Upload data DateTime------------------
void dataTime(String waktu){  
  char waktu_gps[40];
  strcpy(waktu_gps,nameDevice);    
  if(Firebase.RTDB.setString(&fbdo, strcat(waktu_gps,"/raw_data/waktu"), waktu)){
    Serial.println("Data Time Sukses di Upload");
    connStatus = 1;
  }else{
    Serial.print("Error in up Date, ");
    failConnect();
  }
}
//--------------End Fungsi Upload data DateTime--------------
//---------------Fungsi Upload data DateTime------------------
void dataDate(String tanggal){
  char tanggal_gps[40];  
  strcpy(tanggal_gps,nameDevice);
  if(Firebase.RTDB.setString(&fbdo, strcat(tanggal_gps,"/raw_data/tanggal"), tanggal)){
    Serial.println("Data Date Sukses di Upload");
    connStatus = 1;
  }else{
    Serial.print("Error in up Date, ");
    failConnect();
  }
}
//--------------End Fungsi Upload data DateTime--------------
//------------Fungsi Setting Upload data alat----------------
void deviceData(int battery_value, String fingger_data){
  char data_fingger[60], baterai_level[40];  
  strcpy(baterai_level,nameDevice);
  strcpy(data_fingger,nameDevice);    
  if(Firebase.RTDB.setInt(&fbdo, strcat(baterai_level,"/raw_data/battery_level"), battery_value)&&
     Firebase.RTDB.setString(&fbdo, strcat(data_fingger,"/raw_data/fingger_data"), fingger_data)){      
      Serial.println("Data Arduino Sukses di Upload");
      connStatus = 1;
  }else{
    Serial.print("Error in up Arduino Data, ");
    failConnect();
  }  
}
//------------End Fungsi Setting Upload data alat----------------
//------------Fungsi Read data Setting Fingger Alat----------------
void finggerSetup(){
  char status_fingger[40];
  strcpy(status_fingger,nameDevice);
  strcat(status_fingger,"/flag_status/status_fingger");
  if(Firebase.RTDB.get(&fbdo,status_fingger)){
    if(fbdo.stringData().startsWith("enfinger")){          
      wifiSerial.println(fbdo.stringData());      
      Serial.println("enroll finger");
      delay(1000);
      Firebase.RTDB.setDouble(&fbdo,status_fingger, 0);               
    }else if(fbdo.stringData().startsWith("delfinger")){          
      wifiSerial.println(fbdo.stringData());      
      Serial.println("delete finger");
      delay(1000);
      Firebase.RTDB.setDouble(&fbdo,status_fingger, 0);               
    }  
  }else{
    Serial.print("Error in get Fingger, ");
    failConnect();
  } 
}
//----------End Fungsi Read data Setting Fingger Alat-------------
//--------------Fungsi Status Alat--------------------
void deviceStatus(){
  //DEV = Device
  lcd.clear();  
  if(connStatus == 0 && gpsStatus == 0){
    lcd.setCursor(0,0);
    lcd.print("Connecting....  ");
  }else if(connStatus == 1 && gpsStatus == 0){
    lcd.setCursor(0,0);
    lcd.print("DEV : ON");
    //gps status
    lcd.setCursor(0,1);
    lcd.print("GPS : OFF");
  }else if(connStatus == 1 && gpsStatus == 1){
    lcd.setCursor(0,0);
    lcd.print("DEV : ON");
    //gps status
    lcd.setCursor(0,1);
    lcd.print("GPS : ON ");
  }else if(connStatus == 2){
    lcd.setCursor(0,0);
    lcd.print("Wifi Disconnect ");  
    lcd.setCursor(0,1);
    lcd.print("Check AP Device ");
  }
}
//--------------End Fungsi Status Alat--------------------

void loop() {  
  timeGpsData();
  deviceStatus();      
  //--Read data fingger dari firebase dan mengirimkan ke arduino--
  finggerSetup();
  //--End Read data dari firebase dan mengirimkan ke arduino--
  //--Read data parsing dari arduino--
  while(wifiSerial.available()>0) {
  //wifiSerial.setTimeout(50);
    char inChar = (char)wifiSerial.read();
    dataIn += inChar;
    if (inChar == '\n'){
      parsing = true;
    }
//      else if(inChar == 'S'){      
//      Serial.println("Scan Fingger");
//      lcd.setCursor(0,0);
//      lcd.print("Scan Sidik Jari ");           
//      yield();
//    }
  }
  while(pulseSerial.available()>0){
    char inChar2 = (char)pulseSerial.read();
    dataIn2 += inChar2;
    if (inChar2 == '\n'){
      parsing2 = true;
    }
  }  
  if(parsing){
    parsingData();    
    parsing=false;
    dataIn="";       
  }         
  if(parsing2){
    parsingData2();    
    parsing2=false;
    dataIn2="";       
  }
  //--End Read data parsing dari arduino--
}

//----------------------Fungsi PARSING DATA---------------------------  
void parsingData(){
 int j=1;    
//kirim data yang telah diterima sebelumnya
//  Serial.print("DATA DITERIMA : ");
//  Serial.print(dataIn);
  //--PARSING SELURUH DATA--
  dt[j]="";
  for(i=1;i<dataIn.length();i++){
    if ((dataIn[i] == ':')){
      j++;
      dt[j]="";
    }
    else{
      dt[j] = dt[j] + dataIn[i];
    }
  }
  //--PRINT DATA YANG TELAH DI PARSING--
  Serial.println("--------------Parsing Data----------------");
  Serial.print("Data 1 (flag) : ");
  Serial.println(dt[1]);
  Serial.print("Data 2 (battery) : ");
  Serial.println(dt[2].toInt());  
  Serial.print("Data 4 (button value) : ");
  Serial.println(dt[3].toInt());
  Serial.print("Data 5 id finger : ");
  Serial.println(dt[4]);
  Serial.print("Data 6 (jam) : ");
  Serial.println(dt[5].toInt());   
  Serial.print("Data 7 (menit) : ");
  Serial.println(dt[6].toInt());  
  //--KIRIM DATA KE FIREBASE--
  flagStatus(dt[3].toInt());
  deviceData(dt[2].toInt(),dt[4]); 
}

void parsingData2(){
 int k=1;    
//kirim data yang telah diterima sebelumnya
//  Serial.print("DATA DITERIMA : ");
//  Serial.print(dataIn);
  //--PARSING SELURUH DATA--
  dt2[k]="";
  for(l=1;l<dataIn2.length();l++){
    if ((dataIn2[l] == ':')){
      k++;
      dt2[k]="";
    }
    else{
      dt2[k] = dt2[k] + dataIn2[l];
    }
  }
  //--PRINT DATA YANG TELAH DI PARSING--
  Serial.println("--------------Parsing Data----------------");
  Serial.print("Data 1 (flag) : ");
  Serial.println(dt2[1]);
  Serial.print("Data 2 (BPM) : ");
  Serial.println(dt2[2].toInt());
  Serial.print("Data 3 (SPO2) : ");
  Serial.println(dt2[3].toInt());  
  Serial.print("Data 4 (maxBpm) : ");
  Serial.println(dt2[4].toInt());
  Serial.print("Data 5 (minBpm) : ");
  Serial.println(dt2[5].toInt());
  Serial.print("Data 6 (maxSpo) : ");
  Serial.println(dt2[6].toInt());   
  Serial.print("Data 7 (minSpo) : ");
  Serial.println(dt2[7].toInt());  
  //--KIRIM DATA KE FIREBASE--
  dataPulse(dt2[2].toInt(),dt2[3].toInt()); 
}
//----------------------End Fungsi PARSING DATA---------------------------
//----------------------Fungsi get Time dari Ntp---------------------------
void timeNtp(){
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;  
  int currentMonth = ptm->tm_mon+1; 
  int currentYear = ptm->tm_year+1900; 
  
  //Print complete date:
  String currentDate = String(monthDay)+ "/" +String(currentMonth)+ "/" + String(currentYear);
  Serial.println(timeClient.getFormattedTime());
  Serial.println(currentDate);
}
  
//----------------------Fungsi Get Waktu RTC GPS--------------------------  
void timeGpsData(){
  while(gpsSerial.available()>0){
    if (gps.encode(gpsSerial.read())) {
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
    }
  }
  if(millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println("No GPS detected: check wiring.");
  }
  // -- Delete this section if not displaying time ------- //
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {
      prevDisplay = now();
      Serial.println("---------------Data GPS----------------");
      tampilkan();      
    }
  }
}
//----------------------End Fungsi Get Waktu RTC GPS------------------------ 
//-------------------Fungsi Show Waktu RTC dan GPS--------------------------
void tampilkan() {   
  if(gps.location.isValid()){
    latVar = gps.location.lat();
    lngVar = gps.location.lng();
    
    Serial.print("Location = ");
    Serial.print(latVar, 9);
    Serial.print(",");
    Serial.println(lngVar, 9);
    gpsStatus = 1;
    dataGPS(latVar,lngVar);
  }else{
    Serial.println("Lokasi Belum Terbaca");
    gpsStatus = 0;
  }

  if(gps.time.isValid()){    
    sprintf (wjam, "%02d:%02d:%02d", hour(), minute(), second());
    Serial.print("Waktu = ");
    Serial.println(wjam);
    dataTime(wjam);    
  }else{
    Serial.println("Waktu Belum Terbaca");
  }

  if(gps.date.isValid()){
    sprintf (wtanggal, "%02d/%02d/%02d", day(), month(), year());
    Serial.print("Tanggal :");
    Serial.println(wtanggal);
    dataDate(wtanggal);
  }else{
    Serial.println("Tanggal Belum Terbaca");
  }  
}
//--------------------End Fungsi Show Waktu RTC dan GPS------------------------

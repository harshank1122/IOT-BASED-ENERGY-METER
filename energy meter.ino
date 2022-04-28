#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#define FIREBASE_HOST "garbage-3d535-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "02g2yXRKl9g1phkylqJCbelBh2ls11PGU8ENCgmp"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h> 
#include <PZEM004T.h>
int button= 12; 
FirebaseData firebaseData;
PZEM004T pzem(D1,D2);  // RX,TX (D2, D1) on NodeMCU 
IPAddress ip(192,168,1,1);
int ledPower = D7;
#define ON_Board_LED 2  
const char* ssid = "harshank"; 

const char* password = "10203040"; 
const char* host = "script.google.com";
const char* host1 = "maker.ifttt.com";     //IFTTT channel address
int buttonState = HIGH;
const int httpsPort = 443;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
WiFiClientSecure client; 
String GAS_ID = "AKfycbwNhcVRs0VeDj_aQXmfKcF5vb1-t3rb-45shxn_-wO8MP9LOlvtqo80bvUY_ep3izfR9g";
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"}; 

void setup() {
  Serial.begin(115200);
  delay(500);
  pzem.setAddress(ip);
  delay(500);
  WiFi.begin(ssid, password); 
  Serial.println("");
  pinMode(ON_Board_LED,OUTPUT); 
  digitalWrite(ON_Board_LED, HIGH);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
}
   pinMode(button, INPUT);
   digitalWrite(ON_Board_LED, HIGH); 
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // connect to firebase
  Firebase.reconnectWiFi(true);
  pinMode(ledPower, OUTPUT);
  //Firebase.reconnectWiFi(true);
  Serial.println(WiFi.localIP());
  Serial.println();
  client.setInsecure();
  timeClient.begin();
  timeClient.setTimeOffset(19800);
}
int value = 0;
void loop() {

  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

 String weekDay = weekDays[timeClient.getDay()];
 struct tm *ptm = gmtime ((time_t *)&epochTime); 
int monthDay = ptm->tm_mday;
int currentMonth = ptm->tm_mon+1;
String currentMonthName = months[currentMonth-1];
int currentYear = ptm->tm_year+1900;

if (Firebase.getInt(firebaseData, "/red")) {
    if  (firebaseData.dataType() == "int") {
      int val = firebaseData.intData();
      if(val == 1){
    digitalWrite(ledPower, HIGH);
     Serial.println("cled");
  } else {
    digitalWrite(ledPower, LOW);
     Serial.println("conned");
  }
      
    }
  }


   int v = pzem.voltage(ip);
  if(v >= 0.0){ Serial.println(v); }
   
  float i = pzem.current(ip);
  if(i >= 0.0){ Serial.println(i); }
   
  float tem = pzem.power(ip);
  if(tem >= 0.0){ Serial.println(tem); }

  float hum = pzem.energy(ip);
  if(hum >= 0.0){ Serial.println(hum); }

  Firebase.setInt(firebaseData, "/voltage", v);
  Firebase.setInt(firebaseData, "/current", i);
  Firebase.setInt(firebaseData, "/watt", tem);
  Firebase.setInt(firebaseData, "/watthours", hum);
    
  if ((currentSecond == 2 || currentSecond == 3 || currentSecond == 4 || currentSecond == 5 || currentSecond == 6 || currentSecond == 7 || currentSecond == 8 ) && (currentMinute == 20)) {
  //if (currentMinute == 38) {
    //   if (currentSecond == 2) {
sendData(tem, hum);
delay (1000); 
Serial.print(formattedTime);



}


buttonState = digitalRead(button);

    if (buttonState == LOW) {                     //button is pulled down to ground via 10k resistor

       if (value == 0){

         WiFi.disconnect();

        Serial.println("Button Pressed");

       WiFi.begin(ssid, password); // connecting to wifi

       while (WiFi.status() != WL_CONNECTED) {

        Serial.println("Conencting");

       delay(500);

       }

       
       WiFiClient client;

       const int httpPort = 80;

       if (!client.connect(host1, httpPort)) {

        Serial.println("Weak Network");

       return;

       }

       // We now create a URI for the request

       String url = "/trigger/energymeter/json/with/key/eoUnOGRNKaaXryMP30kZp_X7sW92r4QhSCpagNY0wia";   //our link to trigger the event with special key and event name 

  

       // This will send the request to the server

       client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host1 + "\r\n" + "Connection: close\r\n\r\n");  // GET request 

       

       value = 1;

       delay(5000);

    

       }

    }

    else{

    value = 0;

    delay(500);

    }

}


void sendData(float tem, float hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //String string_temperature =  String(tem);
  String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();

} 

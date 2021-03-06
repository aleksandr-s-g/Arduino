#include <Arduino_JSON.h>

int incomingByte = 0;
#define RXD2 16
#define TXD2 17
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include "time.h"
#include <DHT.h>
#define DHTTYPE    DHT11     // DHT 11
#define DHTPIN 4     // Digital pin connected to the DHT sensor

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*3600;
const int   daylightOffset_sec = 0;
const char *ssid = "DIR300";
const char *password = "89538884421";
int curTemp=0;
int curWeather = 0;
String curIcon = "0000";
int curWind = 0;
int curHum = 0;
String curRGBB = ";000;010;000;100";
String curMode = "1;";
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
#define EEPROM_SIZE 10



String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    return "--";
  }
  else {
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    return "--";
  }
  else {
    return String(h);
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial1.begin(1000, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32-clock")) {
    Serial.println("MDNS responder started");
  }
  
  
  struct tm timeinfo;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  dht.begin();


  server.on("/", handleRoot);
  server.on("/th", handleTH);
  server.on("/setmode", setCurMode);
  server.on("/getmode", getCurMode);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");  
  
  
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  get_weather();
  delay(500);
  send_info();

}



void handleRoot() {
    server.send(404, "text/plain", "there is nothing");
}

void checkWifi(){
   if (WiFi.status() == WL_CONNECTED) {
    return;
  }
   WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
}


void handleTH() {
  String answer = "";
  answer = answer + "{\"temp\":\"";
  answer = answer + readDHTTemperature();
  answer = answer + "\",\"hum\":\"";
  answer = answer + readDHTHumidity();
  answer = answer + "\"}";
  server.send(200, "text/plain", answer);
}

void setCurMode() {

    for (uint8_t i = 0; i < server.args(); i++) {
   // message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    if (server.argName(i) == "mode;") curMode = server.arg(i);
    if (server.argName(i) == ";rgbb") curRGBB = server.arg(i);
  }
  
  send_info();
 server.send(200, "text/plain", "OK");
}

void getCurMode()
{
  String answer = "";
  answer = answer + "{\"mode\":\"";
  answer = answer + curMode;
  answer = answer + "\",\"rgbb\":\"";
  answer = answer + curRGBB;
  answer = answer + "\"}";
  server.send(200, "text/plain", answer);
}




void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}



void get_weather()
{
  Serial.println(readDHTTemperature());
  HTTPClient http;
  http.begin("http://api.openweathermap.org/data/2.5/weather?id=1496747&appid=eb9d862de23f0b60cba91a19e5b1b931"); //HTTP
  int httpCode = http.GET();
  if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                // Serial.println(payload);
                JSONVar myWeather = JSON.parse(payload);
                  if (JSON.typeof(myWeather) == "undefined") {
                  Serial.println("Parsing input failed!");
                  }
                  else
                  {
                    // Serial.println("parse success");
                    curTemp = (int)myWeather["main"]["temp"] - 273;
                    // Serial.println(curTemp);
                    
                    curWind = (int)myWeather["wind"]["speed"];
                    curHum = (int)round(((int)myWeather["main"]["humidity"])/4.76);
                    // Serial.println(curHum);
                    /// Serial.println(curWind);
                    // Serial.println(myWeather["weather"][0]["icon"]);
                    curIcon = myWeather["weather"][0]["icon"];
                    curIcon[2] = 0;
                    // Serial.println(curIcon);
                  }
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
}


void send_info()
{
  String info1 = "1;10;11;12;+;05;10;05;14";
  // String info2 = "1;19;44;06;-;14;04;02;14;000;000;255;100";
  //1;19;44;06;+;14;04;02;14;000;000;255;100
  //2;19;17;01;+;16;04 ;02;12;000;000;255;100
  //01234567890123456789012345678901234567890
  //00000000001111111111222222222233333333334
  //mode;hour;min;sec;sign;temp;icon;wind;hum;r;g;b;bright
  
  String curTime = "";
    struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int now_hour = timeinfo.tm_hour;
  int now_min = timeinfo.tm_min;
  int now_sec = timeinfo.tm_sec;
Serial.print(now_hour);
Serial.print(":");
Serial.print(now_min);
Serial.print(":");
Serial.print(now_sec);
Serial.println(":");
//String info = "1;"+String(now_hour)+";"+String(now_min)+";"+String(now_sec)+";+;05;10;05;14"
String info = curMode;
if(now_hour< 10)
{
  info = info + "0" + String(now_hour) + ";";
}
else
{
  info = info + String(now_hour)+ ";";
}

if(now_min< 10)
{
  info = info + "0" + String(now_min) + ";";
}
else
{
  info = info + String(now_min)+ ";";
}

if(now_sec< 10)
{
  info = info + "0" + String(now_sec) + ";";
}
else
{
  info = info + String(now_sec)+ ";";
}
if (curTemp >= 0)
{
  info = info + "0;";
}
else
{
  info = info + "1;";
}
if(abs(curTemp)< 10)
{
  info = info + "0" + String(abs(curTemp)) + ";";
}
else
{
  info = info + String(abs(curTemp))+ ";";
}
info = info + curIcon[0];
info = info + curIcon[1] + ";";
if(curWind< 10)
{
  info = info + "0" + String(curWind) + ";";
}
else
{
  info = info + String(curWind)+ ";";
}

if(curHum< 10)
{
  info = info + "0" + String(curHum);
}
else
{
  info = info + String(curHum);
}



info = info + curRGBB;

 Serial1.print(info); //Write the serial data
 Serial.println(info);
}


void loop() {
  struct tm timeinfo;
    server.handleClient();
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

if( timeinfo.tm_sec == 0)
{

  get_weather();
  send_info();
  checkWifi();
}
}

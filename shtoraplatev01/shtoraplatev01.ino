#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include "time.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTTYPE    DHT11     // DHT 11
#define DHTPIN 21     // Digital pin connected to the DHT sensor
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 6*3600;
const int   daylightOffset_sec = 3600;

#define EEPROM_SIZE 10
/*EEPROM MAP
 * 0 - target percent
 * 1
 * 2
 * 3 - curpos
 * 
  4 - int close_hour = 
  5 - int close_min = 
  6 - int open_hour = 
  7 - int open_min = 
  8 - int close_target = 
  9 - int open_target = 
 */


const char *ssid = "DIR300";
const char *password = "89538884421";

WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

#define NO 0
#define UP 1
#define DOWN 2
const int upPin = 2;
const int downPin = 4;// номер контакта для кнопки
const int upBtnPin = 23;// номер контакта для кнопки
const int downBtnPin = 22;// номер контакта для кнопки
const int ReedS1Pin = 34;// номер контакта для кнопки
const int ReedS2Pin = 26;// номер контакта для 

int upBtnState = 0;
int downBtnState = 0;
int oldUpBtnState = 0;
int oldDownBtnState = 0;
int upBtnPressedTime = 0;
int upBtnReleasedTime = 0;
int upBtnPressedDur = 0;
int downBtnPressedTime = 0;
int downBtnReleasedTime = 0;
int downBtnPressedDur = 0;
int upBtnAction = 0;
int downBtnAction = 0;
int curPercentPos = 10000000;
int targetPercentPos = 0;
int curPos = 10000000;
int tmpCurPos = 10000000;
int maxPos = 20000;
int minPos = -1500;
int startMovingTime = 0;
int oldMovingState = NO;
int movingState = NO;
int doubleClickPause = 500;
int downDoubleClickFlag = 0;
int upDoubleClickFlag = 0;
int onZeroFlag = 0;
// переменная для хранения статуса кнопки:
int buttonState = 0;



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



void set_percent_target(int newPercentTarget)
{
        targetPercentPos = newPercentTarget;
        EEPROM.write(0, targetPercentPos);
        EEPROM.commit();
}

void handleRoot() {
    server.send(404, "text/plain", "there is nothing");
}



void handleEngine() {
  String message = "Params - >";
  int targetPos = -1;
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    if (server.argName(i) == "target") targetPos = server.arg(i).toInt();
  }
  Serial.print ("target = ");
  Serial.println (targetPos);
  if(targetPos > 100) targetPos = 100;
  if(targetPos < 0) targetPos = 0;

  set_percent_target(targetPos);
  // if(targetPos == 0) targetPercentPos = -10;
  server.send(200, "text/plain", String(tmpCurPos));
}

void handleSchedule() {
  /*
   *   4 - int close_hour = 
  5 - int close_min = 
  6 - int open_hour = 
  7 - int open_min = 
  8 - int close_target = 
  9 - int open_target = 
   */
  int close_hour = 22;
  int close_min = 0;
  int open_hour = 7;
  int open_min = 0;
  int close_target = 100;
  int open_target = 0;
  String message = "Params - >";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    if (server.argName(i) == "close_hour") close_hour = server.arg(i).toInt();
    if (server.argName(i) == "close_min") close_min = server.arg(i).toInt();
    if (server.argName(i) == "open_hour") open_hour = server.arg(i).toInt();
    if (server.argName(i) == "open_min") open_min = server.arg(i).toInt();
    if (server.argName(i) == "close_target") close_target = server.arg(i).toInt();
    if (server.argName(i) == "open_target") open_target = server.arg(i).toInt();
  }
  if(close_target > 100) close_target = 100;
  if(close_target < 0) close_target = 0;
  if(open_target > 100) open_target = 100;
  if(open_target < 0) open_target = 0;
  

  EEPROM.write(4, close_hour);
  EEPROM.write(5, close_min);
  EEPROM.write(6, open_hour);
  EEPROM.write(7, open_min);
  EEPROM.write(8, close_target);
  EEPROM.write(9, open_target);
  EEPROM.commit();
  // if(targetPos == 0) targetPercentPos = -10;
  server.send(200, "text/plain", String(tmpCurPos));
}

void handlePos() {
  String answer = "";
  if (targetPercentPos == 0)
  {
    answer = "1";
  }
  else
  {
    answer = "0";
  }
  server.send(200, "text/plain", answer);
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

void setup() {
  Serial.begin(115200);  



  // инициализируем контакт кнопки, делая его входным контактом:
  // инициализируем контакт светодиода, делая его выходным контактом: 
  pinMode(downPin, OUTPUT);
  pinMode(upPin, OUTPUT);
  pinMode(upBtnPin, INPUT);
  pinMode(downBtnPin, INPUT);
  pinMode(ReedS1Pin, INPUT);
  pinMode(ReedS2Pin, INPUT);


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

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/engine", handleEngine);
  server.on("/schedule", handleSchedule);
  server.on("/pos", handlePos);
  server.on("/th", handleTH);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  EEPROM.begin(EEPROM_SIZE);
  targetPercentPos = EEPROM.read(0);
  byte byte0 = EEPROM.read(1);
  byte byte1 = EEPROM.read(2);
  byte byte2 = EEPROM.read(3);
  // Serial.print(byte1);
  // Serial.print("/");
  // Serial.println(byte0);
  int calcTmpCurPos = byte2*255*255 + byte1*255 + byte0;
  tmpCurPos = calcTmpCurPos;
  curPos = calcTmpCurPos;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  dht.begin();
}
void go_down()
{
  Serial.println("go_down");
   digitalWrite(downPin, HIGH);
   digitalWrite(upPin, LOW);
}

void go_up()
{
    Serial.println("go_up");
  digitalWrite(downPin, LOW);
  digitalWrite(upPin, HIGH);
}

void go_stop()
{
    Serial.println("go_stop");

    digitalWrite(downPin, LOW);
  digitalWrite(upPin, LOW);
}
void check_buttons()
{
    int upBtnState = digitalRead(upBtnPin);
    int downBtnState = digitalRead(downBtnPin);
    int now = millis();

    if (digitalRead(ReedS1Pin) == 1|| digitalRead(ReedS2Pin) == 1) 
    {
        onZeroFlag = 1;
    }
    else 
    {
        onZeroFlag = 0;
    }

    
    if(upBtnState == 1 && oldUpBtnState == 0)
    {
      delay (50);
      oldUpBtnState = 1;
      // movingState = UP;
        curPercentPos = tmpCurPos*100/maxPos;
      set_percent_target(curPercentPos);
     if ( now - upBtnReleasedTime < doubleClickPause)
      {
        upDoubleClickFlag = 1;
      }
      upBtnPressedTime = now;  
    }
    if(upBtnState == 0 && oldUpBtnState == 1)
    {
      delay (50);
      oldUpBtnState = 0;
      
      upBtnReleasedTime = now;
      upBtnPressedDur = now - upBtnPressedTime;
      if (upDoubleClickFlag == 1){
        upDoubleClickFlag = 0;
        set_percent_target(0);
        return;
      }
      // movingState = NO;
    }

    if(downBtnState == 1 && oldDownBtnState == 0)
    {
      delay (50);
      oldDownBtnState = 1;
      curPercentPos = tmpCurPos*100/maxPos;
      set_percent_target(curPercentPos);
      // movingState = DOWN;
      if ( now - downBtnReleasedTime < doubleClickPause)
      {
        downDoubleClickFlag = 1;
      }
      downBtnPressedTime = now;

      
    }
    if(downBtnState == 0 && oldDownBtnState == 1)
    {
      delay (50);
      oldDownBtnState = 0;
      downBtnReleasedTime = now;
      downBtnPressedDur = now - downBtnPressedTime;
      if (downDoubleClickFlag == 1){
        downDoubleClickFlag = 0;
        set_percent_target(100);
        return;
      }
      // movingState = NO;

    }
    
}

void time_manager()
{

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int now_hour = timeinfo.tm_hour;
  int now_min = timeinfo.tm_min;
  int now_sec = timeinfo.tm_sec;
  if (now_sec!=0 ) return;
  int close_hour = EEPROM.read(4);
  int close_min = EEPROM.read(5);
  int open_hour = EEPROM.read(6);
  int open_min = EEPROM.read(7);
  int close_target = EEPROM.read(8);
  int open_target = EEPROM.read(9);

  if (now_hour == open_hour && now_min == open_min)
  {
    set_percent_target(open_target);
  }
  
  if (now_hour == close_hour && now_min == close_min)
  {
    set_percent_target(close_target);
  }

}
void engine_handler()
{
  byte byte0 = tmpCurPos % 255;
  byte byte1 = ((tmpCurPos - byte0)/255)%255;
  byte byte2 = ((tmpCurPos - byte0 - byte1)/(255*255))%255;
  EEPROM.write(1, byte0);
  EEPROM.write(2, byte1);
  EEPROM.write(3, byte2);
  EEPROM.commit();
  // Serial.print(byte1);
  // Serial.print("/");
  // Serial.println(byte0);
  int calcTmpCurPos = byte2*255*255 + byte1*255 + byte0;
  // printing status
  Serial.print(calcTmpCurPos);
  Serial.print("/");
  Serial.print(tmpCurPos);
  Serial.print("/");
  curPercentPos = tmpCurPos*100/maxPos;
  Serial.print(curPercentPos);
  Serial.print("/");
  Serial.print(targetPercentPos);
  Serial.print("/");
  if(targetPercentPos == 0 && onZeroFlag == 0 && tmpCurPos > minPos)
  {
    movingState = UP;
    Serial.println("UP");
    return;
  }
  if (curPercentPos > targetPercentPos)
  {
    movingState = UP;
    Serial.println("UP");
    return;
  }
  
  if (curPercentPos == targetPercentPos)
  {
    movingState = NO;
    Serial.println("NO");
    return;
  }
  
  if (curPercentPos < targetPercentPos)
  {
    movingState = DOWN;
    Serial.println("DOWN");
    return;
  }
  
}
void engine_move()
{
  int now = millis();

  if (tmpCurPos > maxPos && movingState == DOWN)
  {
    movingState = NO;
  }

  if (tmpCurPos < minPos && movingState == UP)
  {
    movingState = NO;
  }
// NO
  if(oldMovingState == NO && movingState == NO)
  {
     movingState = NO;
     return;
  }

  if(oldMovingState == NO && movingState == UP)
  {
    oldMovingState = UP;
    startMovingTime = now;
    go_up();
    return;
  }

  if(oldMovingState == NO && movingState == DOWN)
  {
    oldMovingState = DOWN;
    startMovingTime = now;
    go_down();
    return;
  }
 // UP
  
  if(oldMovingState == UP && movingState == NO)
  {
      oldMovingState = NO;
      go_stop();
      int movingDur = now - startMovingTime;
      curPos = curPos - int(movingDur*19/20);
      return;
  }
  
  if(oldMovingState == UP && movingState == UP)
  {
    int movingDur = now - startMovingTime;
    tmpCurPos = curPos - int(movingDur*19/20);

    
    if (onZeroFlag == 1) 
    {
        movingState = NO;
        curPos = 0;
        startMovingTime = now;
    }
    return;
  }
    
  if(oldMovingState == UP && movingState == DOWN)
  {
    int movingDur = now - startMovingTime;
    curPos = curPos - int(movingDur*19/20);
    oldMovingState = DOWN;
    startMovingTime = now;
    go_down();
    return;
  }

// DOWN
  
  if(oldMovingState == DOWN && movingState == NO)
  {
      oldMovingState = NO;
      go_stop();
      int movingDur = now - startMovingTime;
      curPos = curPos + movingDur;
      return;
  }
  if(oldMovingState == DOWN && movingState == UP)
  {
      int movingDur = now - startMovingTime;
      curPos = curPos + movingDur;
      oldMovingState = UP;
      startMovingTime = now;
      go_up();
      return;
  }
  if(oldMovingState == DOWN && movingState == DOWN)
  {
        int movingDur = now - startMovingTime;
        tmpCurPos = curPos + movingDur;
        return;
  }
  
}




void loop() {
  // считываем значение кнопки:
  // Serial.print(".");
  // Serial.print(digitalRead(ReedS1Pin));
  // Serial.print(digitalRead(ReedS2Pin));
  // Serial.print(digitalRead(upBtnPin));
  // Serial.print(digitalRead(downBtnPin));
  // Serial.print(readDHTTemperature());
  // int hallValue = analogRead(hallPin);
  // Serial.println(analogRead(hallPin));
  // int huderedHallValue = hallValue/100;
  // if (hallValue > (hallCenter + hallRadius) || hallValue < (hallCenter - hallRadius)) Serial.println(hallValue);
  // Serial.print ("DCF");
  
  // Serial.println (doubleClickFlag);


  
server.handleClient();
time_manager();
engine_handler();
check_buttons();
engine_move();
}

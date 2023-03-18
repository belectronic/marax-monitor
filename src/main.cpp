//Includes
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>


//Defines
#define SCREEN_WIDTH    128     //Width in px 
#define SCREEN_HEIGHT   64      // Height in px
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C    // or 0x3D Check datasheet or Oled Display
#define BUFFER_SIZE     32

#define RXD2 16
#define TXD2 17

//Internals
long lastMillis;
int seconds = 0;
int lastTimer = 0;
long Serial1Timeout = 0;
char buffer[BUFFER_SIZE];
int tmp_index = 0;

//Mara Data
String maraData[7];
String* lastMaraData;

//Instances
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  memset(buffer, 0, BUFFER_SIZE);
  lastMaraData = maraData;
  Serial1.write(0x11);
}

void getMaraData()
{
  /*
    Example Data: C1.06,116,124,093,0840,1,0\n every ~400-500ms
    Length: 26
    [Pos] [Data] [Describtion]
    0)      C     Coffee Mode (C) or SteamMode (V)
    -        1.06  Software Version
    1)      116   current steam temperature (Celsisus)
    2)      124   target steam temperature (Celsisus)
    3)      093   current hx temperature (Celsisus)
    4)      0840  countdown for 'boost-mode'
    5)      1     heating element on or off
    6)      0     pump on or off
  */
  while (Serial1.available())
  {
    Serial1Timeout = millis();
    char rcv = Serial1.read();
    if (rcv != '\n')
      buffer[tmp_index++] = rcv;
    else {
      tmp_index = 0;
      Serial.println(buffer);
      char* ptr = strtok(buffer, ",");
      int idx = 0;
      while (ptr != NULL)
      {
        maraData[idx++] = String(ptr);
        ptr = strtok(NULL, ",");
      }
      lastMaraData = maraData;
    }
  }
  if (millis() - Serial1Timeout > 6000)
  {
    Serial1Timeout = millis();
    Serial1.write(0x11);
  }
}

void updateView()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  //HX
  display.setCursor(2, 2);
  display.setTextSize(2);
  display.print(lastMaraData[3].toInt());
  display.setTextSize(1);
  display.print((char)247);
  display.setTextSize(1);
  display.print("C");
  //Pump
  display.setCursor(2, 30);
  display.print("H");
  if (lastMaraData[5].toInt() == 0)
    display.drawCircle(17, 33, 6, WHITE);
  else
    display.fillCircle(17, 33, 5, WHITE);
  display.setCursor(30, 30);
  //Heat
  display.print("P");
  if (lastMaraData[6].toInt() == 0)
    display.drawRect(40, 28, 10, 10, WHITE);
  else
    display.fillRect(40, 28, 10, 10, WHITE);
  //Steam
  display.setCursor(2, 50);
  display.setTextSize(2);
  display.print(lastMaraData[1].toInt());
  display.setTextSize(1);
  display.print((char)247);
  display.setTextSize(1);
  display.print("C");
  display.drawLine(55, 0, 55, 68, WHITE);
  display.setCursor(65, 15);
  display.setTextSize(5);
  //Timer
  if (seconds > 3)
  {
    String actual = String(seconds);
    if (actual.length()< 2)
      actual = "0" + actual;
    display.print(actual);
  }
  else if (lastTimer > 3)
  {
    String last = String(lastTimer);
    if (last.length()< 2)
      last = "0" + last;
    display.print(last);
  }
  else
  {
    display.print("00");
  }
  //Mode
  display.setTextSize(1);
  display.setCursor(120, 2);
  display.print(lastMaraData[0].substring(0, 1));
  display.display();
}

void loop()
{
  getMaraData();
  int pumpState = lastMaraData[6].toInt();
  if (pumpState == 1)
  {
    if ( millis() - lastMillis >= 1000)
    {
      lastMillis = millis();
      ++seconds;
      if (seconds > 99)
        seconds = 0;
    }
  }
  else
  {
    if (seconds != 0)
      lastTimer = seconds;
    seconds = 0;
  }
  updateView();
}

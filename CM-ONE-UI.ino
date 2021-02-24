#include <RTC_SAMD51.h>
#include <DateTime.h>
#include <RTC_SAMD21.h>

#include <Wire.h>

#include <VL53L0X.h>

#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "PBUTTON.h"
#include "THEADER.h"


// Initiate the TFT display
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);
TFT_eSprite safetyspr = TFT_eSprite(&tft);

VL53L0X tof;

RTC_SAMD51 rtc;

#define LCD_BACKLIGHT (72Ul) // Control Pin of LCD

// Time counting for display
bool running = false;
bool safety = true;
bool sleep = false;
bool tofConnected = true;
unsigned long currentTime;
unsigned long checkTime;
unsigned long tofCurrentTime;
unsigned long tofCheckTime;
int level = 1;
int oldLevel;

void checkSleep() {
  checkTime = millis(); 
  if (!sleep) {
    if (checkTime - currentTime > 20000) {
      Serial.print("Time to sleep: ");
      Serial.println(checkTime);
      digitalWrite(LCD_BACKLIGHT, LOW);
      sleep = true;
    }
  }
}

void wakeUp() {
  digitalWrite(LCD_BACKLIGHT, HIGH);
  Serial.print("Waking up: ");
  Serial.println(millis());
  sleep = false;
  currentTime = millis();
}

int getDistance() {
  int distance = constrain(tof.readRangeSingleMillimeters(), 50, 350);
  distance = map(distance, 50,350,178,1);
//  Serial.print("read Dist: ");
//  Serial.println(tof.readRangeSingleMillimeters());
  return distance;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
 
//  while (!Serial)
//  {
//    // Wait for Serial
//  }

  rtc.begin();
  DateTime now = DateTime(F(__DATE__), F(__TIME__));
  Serial.println("adjust time!");
  rtc.adjust(now);

  now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  
  Wire.begin();

  tof.setTimeout(1000);
  if (!tof.init()) {
    Serial.println("Failed to iniitialize ToF Sensor");
    tofConnected = false;
//    while (1) {}
  }
  
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(D0,INPUT_PULLUP);
  pinMode(D4, OUTPUT);
  
  digitalWrite(D4,LOW);
  
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0,0,240,90,TFT_LIGHTGREY);
  
  tft.drawRect(30,230,180,30,TFT_DARKGREY);

  tft.setFreeFont(FSS18);
  tft.setTextColor(TFT_BLUE);
  tft.setTextDatum(MC_DATUM);
  tft.drawXBitmap(90,130,PBUTTON,60,60,TFT_RED);
  tft.drawXBitmap(0,20,THEADER, THEADER_width, THEADER_height,TFT_BLUE);

  spr.createSprite(178,28);
  spr.fillSprite(TFT_WHITE);

  safetyspr.createSprite(240,30);
  safetyspr.fillSprite(TFT_RED);
  safetyspr.setFreeFont(FSSB9);
  safetyspr.setTextColor(TFT_WHITE);
  safetyspr.setTextDatum(MC_DATUM);
  safetyspr.drawString("Door open", 120,305);

  Serial.println("Ready to rock!");
  currentTime = millis();
  
}

void loop()
{
  if (digitalRead(BUTTON_1) == LOW) {
    if (sleep) {
      wakeUp();
      delay(100);
      while(!digitalRead(BUTTON_1));
    }
  }
  if (digitalRead(BUTTON_2) == LOW) {
    running = !running;
    delay(100);
    while(!digitalRead(BUTTON_2));
  }
  if (digitalRead(D0)) {
    safety = false;
    running = false;
  } else {
    safety = true;
  }
  if (!safety) {
    running = false;
    safetyspr.pushSprite(0,290);
  } else {
    tft.fillRect(0,290,240,30,TFT_WHITE);
  }
  if (!sleep && tofConnected) {
    tofCheckTime = millis();
    if (tofCheckTime - tofCurrentTime > 500) {
      level = getDistance();
      if (level >= oldLevel) {
        spr.fillRect(0,0,level,28,TFT_ORANGE);
        spr.pushSprite(31,231);
      } else { 
        spr.fillSprite(TFT_WHITE);
        spr.pushSprite(31,231);
        delay(10);
        spr.fillRect(0,0,level,28,TFT_ORANGE);
        spr.pushSprite(31,231);
      }
      tofCurrentTime = millis();
      oldLevel = level;
    }
  }
  if (running) {
    digitalWrite(D4, HIGH);
  } else {
    digitalWrite(D4,LOW);
  }
//  checkSleep();

// put your main code here, to run repeatedly:
}

#include <RTC_SAMD51.h>
#include <DateTime.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <TFT_eSPI.h>

#include "Free_Fonts.h"
#include "PBUTTON.h"
#include "THEADER.h"

// Initiate the TFT display and Sprites
TFT_eSPI tft;
TFT_eSprite levelspr = TFT_eSprite(&tft);
TFT_eSprite safetyspr = TFT_eSprite(&tft);

VL53L0X tof;
RTC_SAMD51 rtc;

#define LCD_BACKLIGHT (72Ul) // Control Pin of LCD

// INTIALIZING VARIABLES 

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

//                ------      FUNCTIONS     ------            //


//      PRINT Date TIME to Serial   //

void printDateTime() {
  DateTime now = rtc.now();
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
}


//        SLEEP        //

void checkSleep() {
  checkTime = millis(); 
  if (!sleep && !running) {
    if (checkTime - currentTime > 20000) {
      Serial.print("Time to sleep: ");
      Serial.println(checkTime);
      digitalWrite(LCD_BACKLIGHT, LOW);
      safety = false;
      sleep = true;
    }
  }
}

//      WAKEUP            //

void wakeUp() {
  digitalWrite(LCD_BACKLIGHT, HIGH);
  Serial.print("Waking up: ");
  Serial.println(millis());
  sleep = false;
  checkforSensor();
  currentTime = millis();
}

void runContinuous() {
  running = !running;
  currentTime = millis();
  tft.fillCircle(220,120,5,TFT_WHITE);
}

void runTimed(int timer) {
  rtc.disableAlarm(0);
  rtc.setAlarm(0,rtc.now() + timer);
  rtc.enableAlarm(0, rtc.MATCH_YYMMDDHHMMSS);
  printDateTime();
  Serial.print("Running for ");
  Serial.print(timer);
  Serial.println(" seconds");
  tft.fillCircle(220,120,5,TFT_GREEN);
  running = true;
}

//      TIMED STOPPING    //

void timedStop(uint32_t flag)
{
  Serial.println("Timed Stop at ");
  printDateTime();
  running = !running;
  tft.fillCircle(220,120,5,TFT_WHITE);
  rtc.disableAlarm(0);
  currentTime = millis();
}

//      LEVEL SENSOR      //

int getDistance() {
  int distance = constrain(tof.readRangeSingleMillimeters(), 50, 350);
  distance = map(distance, 50,350,178,1);
  return distance;
}

void checkforSensor() {
  byte i2cerr;
  Wire.beginTransmission(41);
  i2cerr = Wire.endTransmission();
  if (i2cerr != 0) {
    tofConnected = false;
  }
}

//                ------      SETUP     ------            //

void setup()
{
  
  
  Serial.begin(115200);
//  while (!Serial) { }

  Wire.begin();

//  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
//  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(D0,INPUT_PULLUP);
  pinMode(D4, OUTPUT);
  
  digitalWrite(D4,LOW);

  // Set time and date to last compile time   // 
  
  rtc.begin();
//  DateTime now = DateTime(F(__DATE__), F(__TIME__));
//  Serial.println("adjust time on boot!");
//  rtc.adjust(now);

  printDateTime();
 
  rtc.attachInterrupt(timedStop); // configure TimedStop as interrupt for timer


  //      INTIALIZE TOF Sensor    //

  checkforSensor();
  if (tofConnected) {
    tof.setTimeout(1000);
    if (!tof.init()) {
      Serial.println("Failed to iniitialize ToF Sensor");
      tofConnected = false;
    }
  }
  
  //      INITIALIZE TFT          //
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_WHITE);

  // Draw header
  tft.fillRect(0,0,240,90,TFT_LIGHTGREY);
  tft.drawXBitmap(0,30,THEADER, THEADER_width, THEADER_height,TFT_BLUE);  
  
  
  // Draw SENSOR frame
  tft.drawRect(30,230,180,30,TFT_DARKGREY);
  

  // Draw Level indicator Sprite
  levelspr.createSprite(178,28);
  levelspr.fillSprite(TFT_WHITE);
  if (!tofConnected) {
    levelspr.setFreeFont(FSSB9);
    levelspr.setTextColor(TFT_RED);
    levelspr.setTextDatum(MC_DATUM);
    levelspr.drawString("Sensor Error",89,14);
    levelspr.pushSprite(31,231);  
  }

  // Draw Safety Sprite
  safetyspr.createSprite(240,30);
  safetyspr.fillSprite(TFT_RED);
  safetyspr.setFreeFont(FSSB9);
  safetyspr.setTextColor(TFT_WHITE);
  safetyspr.setTextDatum(MC_DATUM);
  safetyspr.drawString("Door open", 120,305);

  currentTime = millis();
  Serial.println("Ready to rock!");
}

//                ------      MAIN LOOP     ------            //

void loop()
{
  //    CHECK DOOR SWITCH AND SET SAFETY FLAG   //
  
  if (digitalRead(D0)) {
    safety = false;
    running = false;
//    currentTime = millis();
//    wakeUp();
  } else {
    safety = true;
  }

  //    DRAW DOOR WARNING BASED ON SAFETY FLAG   //
  
  if (!safety) {
    running = false;
    safetyspr.pushSprite(0,290);
  } else {
    tft.fillRect(0,290,240,30,TFT_WHITE);
  }


  //    CHECK RUNNING FLAG STATE AND CONTROL RELAY    //
  
  if (running) {
    digitalWrite(D4, HIGH);
    tft.drawXBitmap(90,130,PBUTTON,60,60,TFT_GREEN);
  } else {
    digitalWrite(D4,LOW);
    rtc.disableAlarm(0);
    tft.drawXBitmap(90,130,PBUTTON,60,60,TFT_RED);
  }

  //    MONITOR LEVEL SENSOR AND SHOW LEVEL   //

  if (!sleep && tofConnected) {
    checkforSensor();
    tofCheckTime = millis();
    if (tofCheckTime - tofCurrentTime > 500) {
      level = getDistance();
      if (level >= oldLevel) {
        levelspr.fillRect(0,0,level,28,TFT_ORANGE);
        levelspr.pushSprite(31,231);
      } else { 
        levelspr.fillSprite(TFT_WHITE);
        levelspr.pushSprite(31,231);
        delay(10);
        levelspr.fillRect(0,0,level,28,TFT_ORANGE);
        levelspr.pushSprite(31,231);
      }
      tofCurrentTime = millis();
      oldLevel = level;
    }
  } else if (!tofConnected) {
    levelspr.fillSprite(TFT_WHITE);
    levelspr.setFreeFont(FSSB9);
    levelspr.setTextColor(TFT_RED);
    levelspr.setTextDatum(MC_DATUM);
    levelspr.drawString("Sensor Error",89,14);
    levelspr.pushSprite(31,231);  
  }

  //    BUTTON 1 to Wake form Sleep     //
  
  if (digitalRead(BUTTON_1) == LOW) {
    if (sleep) {
      wakeUp();
      delay(100);
      while(!digitalRead(BUTTON_1));
    }
  }

  //    BUTTON 2 or 5WAY press to toggle RUN              //
  
  if (digitalRead(BUTTON_2) == LOW) {
    if (!sleep && safety) {
      runContinuous();
    } else {
      wakeUp();
    }
    delay(100);
    while(!digitalRead(BUTTON_2));
  }

  if (digitalRead(WIO_5S_PRESS) == LOW) {
    if (!sleep && safety) {
      runContinuous();
    } else {
      wakeUp();
    }
    delay(100);
    while(!digitalRead(WIO_5S_PRESS));
  }
  
  //    BUTTON 3 for 30 second timed run    //
  
  if (digitalRead(BUTTON_3) == LOW) {
    if (!sleep && safety) {
      runTimed(30);
    } else {
      wakeUp();
    }
    delay(100);
    while(!digitalRead(BUTTON_3));
  }
  checkSleep();
}

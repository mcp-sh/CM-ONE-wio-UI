#include <TFT_eSPI.h>
#include "Free_Fonts.h"

// Initiate the TFT display
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);
TFT_eSprite safetyspr = TFT_eSprite(&tft);

#define LCD_BACKLIGHT (72Ul) // Control Pin of LCD

// Time counting for display
int fakeLevel = 1;
bool safety = true;
bool sleep = false;
unsigned long currentTime;
unsigned long checkTime;

void checkSleep() {
  checkTime = millis(); 
  if (!sleep) {
    if (checkTime - currentTime > 10000) {
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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0,0,240,90,TFT_LIGHTGREY);
  tft.drawFastHLine(40,75,160,TFT_BLUE);
  tft.drawRoundRect(80,130,80,60,10,TFT_BLUE);
  tft.drawRoundRect(81,131,78,58,9,TFT_BLUE);
  
  tft.drawRect(30,230,180,30,TFT_DARKGREY);

  tft.setFreeFont(FSS18);
  tft.setTextColor(TFT_BLUE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CM-ONE",120,45);
  tft.drawString("ON",120,160);

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
    safety = !safety;
    delay(100);
    while(!digitalRead(BUTTON_2));
  }
  if (!safety) {
    safetyspr.pushSprite(0,290);
  } else {
    tft.fillRect(0,290,240,30,TFT_WHITE);
  }
  fakeLevel++;
  if (fakeLevel = 178) {
    fakeLevel = 1;
  }
  Serial.print("Level: ");
  Serial.println(fakeLevel);
  
 
 
// checkSleep();


  // put your main code here, to run repeatedly:
}

/*
  KCmaster - KETACLOCK master (ESP8266)
  Copyright (C) 2023 Masato YAMANISHI n24bass@gmail.com

  MIT License
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>  // for WIFI
#include <time.h> 

#include <Wire.h>                                      //
#include <Adafruit_GFX.h>                              // OLED
#include <Adafruit_SSD1306.h>                          // OLED 
Adafruit_SSD1306 display(128, 64, &Wire, -1); // reset or (128, 64, &Wire, 4); // 4:OLED_RESET

// Wi-Fi設定
#define WIFI_SSID    "MY_SSID"
#define WIFI_PASSWORD "PASSWORD"

// NTP設定
#define TIMEZONE_JST  (3600 * 9)  // JST (+9)
#define DAYLIGHTOFFSET_JST  (0)   // no summertime
#define NTP_SERVER1   "ntp.nict.jpntp.nict.jp"  // primary NTP server
#define NTP_SERVER2   "ntp.jst.mfeed.ad.jp"   // secondary NTP server

#define DISABLE_CODE_FOR_RECEIVER

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

void setup() {
  Serial.begin(115200);
  Serial.println("START");
    
  // setup and connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // syncronyze to NTP
  configTime(TIMEZONE_JST, DAYLIGHTOFFSET_JST, NTP_SERVER1, NTP_SERVER2);

  // setup LCD
  Wire.begin(13, 14);                                       // I2C SDA,SCL
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);                // I2C ADDRESS=0x3C
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(F("KETACLOCK"));
  display.display();

  // start IR sender with IR_SEND_PIN as send pin and enaable feedback LED at default feedback LED pin
  IrSender.begin(DISABLE_LED_FEEDBACK);

  delay(1000);
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */

void loop() {
  static time_t timeOld = 0;
  static uint8_t secOld = 255;

  // get current datetime
  time_t timeNow = time(NULL); // ms
#if 1 // defined(ESP8266) ?
  if (timeNow == timeOld) {
    delay(10);
  }
#else
  if (timeNow / 1000 == timeOld / 1000) {
    delay(1000 - (timeNow % 1000));
    return;
  }
#endif
  timeOld = timeNow;

  struct tm* tmNow = localtime(&timeNow);

  // --- IRsend

  uint16_t addr = (tmNow->tm_hour << 8) | tmNow->tm_min;
  uint8_t cmd = tmNow->tm_sec;
  if (cmd == secOld) return;
  secOld = cmd;

  IrSender.sendNEC(addr, cmd, 0); // 0:repeat

  // -- LCD
  
  // 曜日文字列配列
  static const char *pszWDay[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  static const char *pszMonth[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  
  // 日付
  char szDate[32];
  sprintf(szDate, "%s %s %02d",
          pszWDay[tmNow->tm_wday],
          pszMonth[tmNow->tm_mon],
          tmNow->tm_mday);
  // 時刻
  char szTime[32];
  sprintf( szTime, " %02d:%02d:%02d",
	   tmNow->tm_hour,
	   tmNow->tm_min,
	   tmNow->tm_sec );
  Serial.println(szTime);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("   * KETA CLOCK *"));
  display.println();
  display.setTextSize(2);
  display.println(szDate);
  display.setTextSize(1);
  display.println(F(""));
  display.setTextSize(2);
  display.println(szTime);
  display.display();
  // delay(100);
}

#include <WiFi.h>
#include <Math.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFiClientSecure.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Preferences.h>

// uncomment next line to use HSPI for EPD (and VSPI for SD), e.g. with Waveshare ESP32 Driver Board
#define USE_HSPI_FOR_EPD

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_7C.h>
#include "Fonts/FreeSansBoldNotoSansWeatherSymbols40pt.h"
#include "Fonts/FreeSansBoldNotoSansWeatherSymbols24pt.h"
#include "Fonts/FreeSansBoldNotoSansWeatherSymbols16pt.h"

#include "Icons/big_weather_icons.h"
#include "Icons/small_weather_icons.h"
#include "img.h"

#include "config.h"

// select the display class (only one), matching the kind of display panel
#define GxEPD2_DISPLAY_CLASS GxEPD2_7C

// select the display driver class (only one) for your  panel
#define GxEPD2_DRIVER_CLASS GxEPD2_565c // Waveshare 5.65" 7-color

// somehow there should be an easier way to do this
#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

WiFiClientSecure client;
HTTPClient http;
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
Preferences config;

class Timezone{
  private:

    const char* _timezoneOlson;
    time_t _lastUpdateTs;
    long _offset = 0;


  public:

    void refresh(time_t openWeatherMapCurrentDt, const char* openWeatherMapTimezone, long openWeatherMapTimezoneOffset){
      _lastUpdateTs = openWeatherMapCurrentDt;
      _timezoneOlson = openWeatherMapTimezone;
      _offset = openWeatherMapTimezoneOffset;
    }

    time_t getLocalTimestamp(time_t ts){
      return ts + _offset;
    }

    time_t getLocalTimestamp(){
      return _lastUpdateTs + _offset;
    }

    time_t getLocalTimestampFloorToUTC(time_t ts){
      return getLocalTimestamp(ts) / 86400 * 86400 - _offset;
    }

    int getTimezoneOffsetHour(){
      return _offset / 60 / 60;
    }

};

Timezone localTimezone;

void printErrorOnScreen(const char* message, int retryDelay){
  display.setFullWindow();
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      

      display.fillCircle((display.width() - DISPLAY_WIDTH_BORDER_OFFSET) / 2, display.height() / 2 - 55, 35, GxEPD_RED);
      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_WHITE);
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols40pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      uint16_t tbw = u8g2_for_adafruit_gfx.getUTF8Width("X");
      u8g2_for_adafruit_gfx.setCursor(((display.width() - DISPLAY_WIDTH_BORDER_OFFSET)- tbw) / 2 + 1, display.height() / 2 - 33);
      u8g2_for_adafruit_gfx.print("X");

      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_BLACK);
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols24pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      tbw = u8g2_for_adafruit_gfx.getUTF8Width(message);
      u8g2_for_adafruit_gfx.setCursor(((display.width() - DISPLAY_WIDTH_BORDER_OFFSET)- tbw) / 2, display.height() / 2 + 20);
      u8g2_for_adafruit_gfx.print(message);

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      char retryMessage[25];
      snprintf(retryMessage, sizeof(retryMessage), "Retrying in %d minutes.", retryDelay);
      tbw = u8g2_for_adafruit_gfx.getUTF8Width(retryMessage);
      u8g2_for_adafruit_gfx.setCursor(((display.width() - DISPLAY_WIDTH_BORDER_OFFSET)- tbw) / 2, display.height() / 2 + 50);
      u8g2_for_adafruit_gfx.print(retryMessage);
      
      display.fillRect(0, 582, display.width() - DISPLAY_WIDTH_BORDER_OFFSET, 18, GxEPD_BLACK);
      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_WHITE);
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(5, 599);
      u8g2_for_adafruit_gfx.printf("Last update: %04d-%02d-%02d %02d:%02d:%02d UTC%+d", year(localTimezone.getLocalTimestamp(now())), month(localTimezone.getLocalTimestamp(now())), day(localTimezone.getLocalTimestamp(now())), hour(localTimezone.getLocalTimestamp(now())), minute(localTimezone.getLocalTimestamp(now())), second(localTimezone.getLocalTimestamp(now())), localTimezone.getTimezoneOffsetHour());
    }
    while (display.nextPage());
    Serial.println("Print done");
    display.powerOff();
    Serial.println("powerOff Done!");
}

#include "Modules/WeatherModule.ino"
WeatherModule myWeatherModule;

#include "Modules/CountdownModule.ino"
CountdownModule myCountdownModule;

#include "Modules/BarclaysModule.ino"
BarclaysModule myBarclaysModule;

#include "Modules/CustomMessagesModule.ino"
CustomMessagesModule myCustomMessagesModule;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // *** special handling for Waveshare ESP32 Driver board *** //
  // ********************************************************* //
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
  hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
  // *** end of special handling for Waveshare ESP32 Driver board *** //
  // **************************************************************** //
  display.init(115200);
  u8g2_for_adafruit_gfx.begin(display); 
  // first update should be full refresh

  display.setRotation(3);
  display.clearScreen();

  Serial.println("");
  Serial.print("Connecting Wifi");
  unsigned long connectMillis = millis();
  WiFi.begin(WIFISSID, WIFIPASSWORD);
  while (WiFi.status() != WL_CONNECTED && millis() - connectMillis < 5000){
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED){
    Serial.print("WiFi Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  client.setInsecure();
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
}


void loop() {

  while(WiFi.status()!= WL_CONNECTED){
    Serial.println("Wi-fi Disconnected");
    printErrorOnScreen("WiFi disconnected", 5);
    delay(100000);
    WiFi.disconnect();
    Serial.print("Connecting Wifi");
    unsigned long connectMillis = millis();
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED && millis() - connectMillis < 5000){
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println("WiFi WL_CONNECTED");

  config.begin("esp32-board", false);

  client.connect(CONFIGURL, 443);
  http.begin(client, CONFIGURL);

  int configHttpResponseCode = http.GET();
  String configResponse = http.getString();

  http.end();
  client.stop();

  Serial.println("ConfigHttpResponseCode: ");
  Serial.println(configHttpResponseCode);
  Serial.println("Config Response: ");
  Serial.println(configResponse);

  if (configHttpResponseCode == 200){
    config.putString("configResponse", configResponse);
  } else {
    Serial.println("Getting config from memory");
    configResponse = config.getString("configResponse");
  }

  config.end();

  JsonDocument configJsonObject;
  deserializeJson(configJsonObject, configResponse);

  if (myWeatherModule.refresh(0, configJsonObject["weather"]) == -1){
    printErrorOnScreen("OpenWeather API Error", 10);
    delay(200000);
    return;
  }

  int yOffset = 305;

  if(configJsonObject["countdown"]["enable"]){
    myCountdownModule.refresh(yOffset, configJsonObject["countdown"]["events"][0]);
    yOffset = yOffset + 100;
  }
  if(configJsonObject["barclays"]["enable"]){ 
    myBarclaysModule.refresh(yOffset);
    yOffset = yOffset + 100;
  }
  if(configJsonObject["customMessages"]["enable"]){
    myCustomMessagesModule.refresh(yOffset, configJsonObject["customMessages"]);
    yOffset = yOffset + 100;
  }
  
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    myWeatherModule.print();

    if(configJsonObject["countdown"]["enable"])
      myCountdownModule.print();

    if(configJsonObject["barclays"]["enable"])
      myBarclaysModule.print();

    if(configJsonObject["customMessages"]["enable"])
      myCustomMessagesModule.print();
    
    display.fillRect(0, 582, display.width() - DISPLAY_WIDTH_BORDER_OFFSET, 18, GxEPD_BLACK);
    u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_WHITE);
    u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setCursor(5, 599);
    u8g2_for_adafruit_gfx.printf("Last update: %04d-%02d-%02d %02d:%02d:%02d UTC%+d", year(localTimezone.getLocalTimestamp()), month(localTimezone.getLocalTimestamp()), day(localTimezone.getLocalTimestamp()), hour(localTimezone.getLocalTimestamp()), minute(localTimezone.getLocalTimestamp()), second(localTimezone.getLocalTimestamp()), localTimezone.getTimezoneOffsetHour());
  }
  while (display.nextPage());
  Serial.println("Print done");
  display.powerOff();
  Serial.println("powerOff Done!");
  delay(UPDATEINTERVAL);
}

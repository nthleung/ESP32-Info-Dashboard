class CountdownModule{

  private:
    long _countdownTargetTimestamp;
    const char* _countdownEventName;
    long _daysRemaining;

  public:

    int yOffset;

    void refresh(int y, JsonObject countdownJsonObject){
      yOffset = y;
      _countdownTargetTimestamp = (long)countdownJsonObject["targetDateTs"];
      _countdownEventName = countdownJsonObject["eventName"];
      _daysRemaining = (_countdownTargetTimestamp - localTimezone.getLocalTimestamp(now())) / 60 / 60 / 24 + 1;
    }

    void print(){
      display.fillRoundRect(0, yOffset, display.width() - DISPLAY_WIDTH_BORDER_OFFSET, 90, 5, GxEPD_GREEN);
      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_WHITE);
      
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols24pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(5, yOffset + 65);
      if (_daysRemaining > 0){
        u8g2_for_adafruit_gfx.print("↓");
      } else {
        u8g2_for_adafruit_gfx.print("↑");
      }

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols40pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(30, yOffset + 65);
      u8g2_for_adafruit_gfx.print(abs(_daysRemaining));

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols24pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(130, yOffset + 40);
      u8g2_for_adafruit_gfx.print(_countdownEventName);

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(130, yOffset + 70);
      u8g2_for_adafruit_gfx.printf("Target date: %d %s %d", day(_countdownTargetTimestamp), monthShortStr(month(_countdownTargetTimestamp)), year(_countdownTargetTimestamp));
    }
};

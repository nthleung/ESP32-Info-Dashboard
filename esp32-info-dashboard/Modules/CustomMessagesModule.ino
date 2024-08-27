class CustomMessagesModule{
  private:

    JsonArray _myCustomMessages;

  public:

    int yOffset;

    void refresh(int y, JsonObject customMessagesConfig){
      yOffset = y;
      _myCustomMessages = customMessagesConfig["messages"];
    }

    void print(){
      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_BLACK);

      display.drawBitmap(2, yOffset + 5, epd_bitmap_feed_icon, epd_bitmap_feed_icon_width, epd_bitmap_feed_icon_height, GxEPD_BLACK);

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);

      int messageYOffset = yOffset;

      for (int i = 0; i < _myCustomMessages.size(); i++){
        messageYOffset += 20;
        u8g2_for_adafruit_gfx.setCursor(45, messageYOffset);
        u8g2_for_adafruit_gfx.print((const char*)_myCustomMessages[i]);
        if (i < _myCustomMessages.size() - 1){
          messageYOffset += 7;
          display.drawRect(45, messageYOffset, display.width() - DISPLAY_WIDTH_BORDER_OFFSET - 45 - 5, 1, GxEPD_BLACK);
          messageYOffset += 3;
        }
      }
    }

};
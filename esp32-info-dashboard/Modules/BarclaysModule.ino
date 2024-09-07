class BarclaysModule{
  private:
    const char* _ticketmasterApiBaseUrl = "https://app.ticketmaster.com/discovery/v2/events?venueId=KovZ917AtP3&sort=date,asc&apikey=" TICKETMASTERAPIKEY;
    char _localTimeString[15] = "";
    char _eventNameLine1[31] = "", _eventNameLine2[31] = "";
    int _refreshCounter = 2;

  public:

    int yOffset;

    void refresh(int y){

      yOffset = y;

      if (_refreshCounter < 2){
        _refreshCounter++;
        return;
      } 

      _refreshCounter = 0;
      memset(_localTimeString, 0, sizeof(_localTimeString));
      memset(_eventNameLine1, 0, sizeof(_eventNameLine1));
      memset(_eventNameLine2, 0, sizeof(_eventNameLine2));

      char startDateTime[31], endDateTime[31];
      snprintf(startDateTime, sizeof(startDateTime), "%04d-%02d-%02dT%02d:%02d:00Z", year(localTimezone.getLocalTimestampFloorToUTC(now())), month(localTimezone.getLocalTimestampFloorToUTC(now())), day(localTimezone.getLocalTimestampFloorToUTC(now())), hour(localTimezone.getLocalTimestampFloorToUTC(now())), minute(localTimezone.getLocalTimestampFloorToUTC(now())));
      snprintf(endDateTime, sizeof(endDateTime), "%04d-%02d-%02dT%02d:%02d:00Z", year(localTimezone.getLocalTimestampFloorToUTC(now()) + 86400), month(localTimezone.getLocalTimestampFloorToUTC(now()) + 86400), day(localTimezone.getLocalTimestampFloorToUTC(now()) + 86400), hour(localTimezone.getLocalTimestampFloorToUTC(now()) + 86400), minute(localTimezone.getLocalTimestampFloorToUTC(now()) + 86400));

      char ticketmasterApiUrl[255];
      strcpy(ticketmasterApiUrl, _ticketmasterApiBaseUrl);
      strcat(ticketmasterApiUrl, "&startDateTime=");
      strcat(ticketmasterApiUrl, startDateTime);
      strcat(ticketmasterApiUrl, "&endDateTime=");
      strcat(ticketmasterApiUrl, endDateTime);

      client.connect(ticketmasterApiUrl, 443);
      http.begin(client, ticketmasterApiUrl);

      int ticketmasterApiHttpResponseCode = http.GET();
      String ticketmasterApiResponse = http.getString();

      http.end();
      client.stop();

      Serial.println("ticketmasterApiHttpResponseCode: ");
      Serial.println(ticketmasterApiHttpResponseCode);
      Serial.println("ticketmasterApi Response: ");
      Serial.println(ticketmasterApiResponse);

      if(ticketmasterApiHttpResponseCode != 200){
        strcpy(_eventNameLine1, "API Error!");
        _refreshCounter = 2;
        return;
      }

      JsonDocument ticketmasterApiJsonObject;
      deserializeJson(ticketmasterApiJsonObject, ticketmasterApiResponse);

      int ticketmasterApiJsonObjectIndex;
      bool hasEvent = false;
      char eventName[63], localStartTime[15], localEndTime[15];
      for (ticketmasterApiJsonObjectIndex = 0; ticketmasterApiJsonObjectIndex < ticketmasterApiJsonObject["_embedded"]["events"].size(); ticketmasterApiJsonObjectIndex++){
        if (ticketmasterApiJsonObject["_embedded"]["events"][ticketmasterApiJsonObjectIndex]["dates"]["start"] && ticketmasterApiJsonObject["_embedded"]["events"][ticketmasterApiJsonObjectIndex]["dates"]["end"]){
          strcpy(eventName, ticketmasterApiJsonObject["_embedded"]["events"][ticketmasterApiJsonObjectIndex]["name"]);
          strcpy(localStartTime, ticketmasterApiJsonObject["_embedded"]["events"][ticketmasterApiJsonObjectIndex]["dates"]["start"]["localTime"]);
          strcpy(localEndTime, ticketmasterApiJsonObject["_embedded"]["events"][ticketmasterApiJsonObjectIndex]["dates"]["end"]["localTime"]);
          hasEvent = true;
          break;
        }
      }

      if (hasEvent){
        snprintf(_localTimeString, sizeof(_localTimeString), "%.5s - %.5s", localStartTime, localEndTime);
      
        char *token;
        token = strtok(eventName, " ");
        int currentLine = 1;

        u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
        u8g2_for_adafruit_gfx.setFontMode(1);

        while( token != NULL ) {
          if (currentLine == 1 && u8g2_for_adafruit_gfx.getUTF8Width(_eventNameLine1) + u8g2_for_adafruit_gfx.getUTF8Width(token) < 230){
            strcat(_eventNameLine1, token);
            strcat(_eventNameLine1, " ");
          } else {
            currentLine = 2;
            if (u8g2_for_adafruit_gfx.getUTF8Width(_eventNameLine2) + u8g2_for_adafruit_gfx.getUTF8Width(token) >= 230){
              break;
            }
            strcat(_eventNameLine2, token);
            strcat(_eventNameLine2, " ");
          }
          token = strtok(NULL, " ");
        }

      } else {
        strcat(_eventNameLine1, "No Event");
      }
    }

    void print(){
      display.fillRoundRect(0, yOffset, display.width() - DISPLAY_WIDTH_BORDER_OFFSET, 90, 5, GxEPD_BLUE);
      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_WHITE);

      display.drawBitmap(5, yOffset + 8, epd_bitmap_barclays, epd_bitmap_barclays_width, epd_bitmap_barclays_height, GxEPD_WHITE);

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      u8g2_for_adafruit_gfx.setCursor(5 + epd_bitmap_barclays_width + 10, yOffset + 25);
      u8g2_for_adafruit_gfx.print(_localTimeString);
      u8g2_for_adafruit_gfx.setCursor(5 + epd_bitmap_barclays_width + 10, yOffset + 50);
      u8g2_for_adafruit_gfx.print(_eventNameLine1);
      u8g2_for_adafruit_gfx.setCursor(5 + epd_bitmap_barclays_width + 10, yOffset + 75);
      u8g2_for_adafruit_gfx.print(_eventNameLine2);
    }
};
class WeatherModule{
  private:

    const char* _openWeatherMapApiBaseUrl = "https://api.openweathermap.org/data/3.0/onecall?exclude=minutely,daily,alerts&units=metric&appid=" OPENWEATHERMAPAPIKEY;
    JsonDocument _openWeatherMapApiJsonObject;
    bool _showFahrenheit = false;
    bool _initialized = false;

    float celsius2fahrenheit (float celsius){
      return celsius * 9 / 5 + 32;
    }

    void printWeatherIcon (int x, int y, const char* icon){
      if (strcmp(icon, "01d") == 0){
        display.drawBitmap(x, y + 10, epd_bitmap_sun, epd_bitmap_sun_width, epd_bitmap_sun_height, GxEPD_RED);
      } else if (strcmp(icon, "01n") == 0) {
        display.drawBitmap(x, y + 10, epd_bitmap_moon, epd_bitmap_moon_width, epd_bitmap_moon_height, GxEPD_ORANGE);
      } else if (strcmp(icon, "02d") == 0 || strcmp(icon, "02n") == 0){
        if (strcmp(icon, "02d") == 0){
          display.fillCircle(x + 20, y + 45, 25, GxEPD_RED);
        } else if (strcmp(icon, "02n") == 0){
          display.fillCircle(x + 20, y + 45, 25, GxEPD_ORANGE);
        }
        display.drawBitmap(x, y + 35, epd_bitmap_cloud, epd_bitmap_cloud_width, epd_bitmap_cloud_height, GxEPD_BLACK);
      } else if (strcmp(icon, "03d") == 0 || strcmp(icon, "03n") == 0 || strcmp(icon, "04d") == 0 || strcmp(icon, "04n") == 0){
        display.drawBitmap(x, y + 25, epd_bitmap_cloud, epd_bitmap_cloud_width, epd_bitmap_cloud_height, GxEPD_BLACK);
      } else if (strcmp(icon, "09d") == 0 || strcmp(icon, "09n") == 0){
        display.drawBitmap(x, y + 10, epd_bitmap_cloud, epd_bitmap_cloud_width, epd_bitmap_cloud_height, GxEPD_BLACK);
        display.drawBitmap(x + 20, y + 80, epd_bitmap_rain, epd_bitmap_rain_width, epd_bitmap_rain_height, GxEPD_BLUE);
      } else if (strcmp(icon, "10d") == 0 || strcmp(icon, "10n") == 0){
        if (strcmp(icon, "10d") == 0){
          printWeatherIcon(x, y - 5, "02d");
        } else if (strcmp(icon, "10n") == 0){
          printWeatherIcon(x, y - 5, "02n");
        }
        display.drawBitmap(x + 20, y + 100, epd_bitmap_halfrain, epd_bitmap_halfrain_width, epd_bitmap_halfrain_height, GxEPD_BLUE);
      } else if (strcmp(icon, "11d") == 0 || strcmp(icon, "11n") == 0){
        display.drawBitmap(x + 32, y + 55, epd_bitmap_thunder, epd_bitmap_thunder_width, epd_bitmap_thunder_height, GxEPD_ORANGE);
        display.drawBitmap(x, y + 10, epd_bitmap_cloud, epd_bitmap_cloud_width, epd_bitmap_cloud_height, GxEPD_BLACK);
      } else if (strcmp(icon, "13d") == 0 || strcmp(icon, "13n") == 0){
        display.drawBitmap(x, y + 10, epd_bitmap_snow, epd_bitmap_snow_width, epd_bitmap_snow_height, GxEPD_BLUE);
      } else if (strcmp(icon, "50d") == 0 || strcmp(icon, "50n") == 0){
        display.drawBitmap(x, y + 25, epd_bitmap_mist, epd_bitmap_mist_width, epd_bitmap_mist_height, GxEPD_BLACK);
      }
    }

    void printHourlyWeather (int x, int y, JsonObject hourlyJsonObject){
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols16pt);
      u8g2_for_adafruit_gfx.setFontMode(1);

      //Print hour text
      time_t t = (long)hourlyJsonObject["dt"];

      char hr[5];
      if (isAM(localTimezone.getLocalTimestamp(t))){
        snprintf(hr, sizeof(hr), "%dam", hourFormat12(localTimezone.getLocalTimestamp(t)));
      } else if (isPM(localTimezone.getLocalTimestamp(t))){
        snprintf(hr, sizeof(hr), "%dpm", hourFormat12(localTimezone.getLocalTimestamp(t)));
      }
      
      uint16_t tbw = u8g2_for_adafruit_gfx.getUTF8Width(hr);

      u8g2_for_adafruit_gfx.setCursor(x + ((display.width() - DISPLAY_WIDTH_BORDER_OFFSET) / 5 - tbw) / 2, y + 20);
      u8g2_for_adafruit_gfx.print(hr);

      printSmallWeatherIcon(x + 10, y + 30, (const char*)hourlyJsonObject["weather"][0]["icon"]);

      //Print temperature and precipitation
      char temperature[6], precipitation[8];

      snprintf(temperature, sizeof(temperature), "%.0f℃", round((float)hourlyJsonObject["temp"]));
      tbw = u8g2_for_adafruit_gfx.getUTF8Width(temperature);
      u8g2_for_adafruit_gfx.setCursor(x + ((display.width() - DISPLAY_WIDTH_BORDER_OFFSET) / 5 - tbw) / 2, y + 115);
      u8g2_for_adafruit_gfx.print(temperature);

      snprintf(precipitation, sizeof(precipitation), "☂%.0f%%", round((float)hourlyJsonObject["pop"] * 100));
      tbw = u8g2_for_adafruit_gfx.getUTF8Width(precipitation);
      u8g2_for_adafruit_gfx.setCursor(x + ((display.width() - DISPLAY_WIDTH_BORDER_OFFSET) / 5 - tbw) / 2, y + 135);
      u8g2_for_adafruit_gfx.print(precipitation);
    }

    void printSmallWeatherIcon (int x, int y, const char* icon){

      if (strcmp(icon, "01d") == 0){
        display.drawBitmap(x + 10, y + 5, epd_bitmap_sun_small, epd_bitmap_sun_small_width, epd_bitmap_sun_small_height, GxEPD_RED);
      } else if (strcmp(icon, "01n") == 0) {
        display.drawBitmap(x + 10, y + 5, epd_bitmap_moon_small, epd_bitmap_moon_small_width, epd_bitmap_moon_small_height, GxEPD_ORANGE);
      } else if (strcmp(icon, "02d") == 0 || strcmp(icon, "02n") == 0){
        if (strcmp(icon, "02d") == 0){
          display.fillCircle(x + 20, y + 27, 15, GxEPD_RED);
        } else if (strcmp(icon, "02n") == 0){
          display.fillCircle(x + 20, y + 27, 15, GxEPD_ORANGE);
        }
        display.drawBitmap(x + 10, y + 22, epd_bitmap_cloud_small, epd_bitmap_cloud_small_width, epd_bitmap_cloud_small_height, GxEPD_BLACK);
      } else if (strcmp(icon, "03d") == 0 || strcmp(icon, "03n") == 0 || strcmp(icon, "04d") == 0 || strcmp(icon, "04n") == 0){
        display.drawBitmap(x + 10, y + 15, epd_bitmap_cloud_small, epd_bitmap_cloud_small_width, epd_bitmap_cloud_small_height, GxEPD_BLACK);
      } else if (strcmp(icon, "09d") == 0 || strcmp(icon, "09n") == 0){
        display.drawBitmap(x + 10, y + 2, epd_bitmap_cloud_small, epd_bitmap_cloud_small_width, epd_bitmap_cloud_small_height, GxEPD_BLACK);
        display.drawBitmap(x + 20, y + 37, epd_bitmap_rain_small, epd_bitmap_rain_small_width, epd_bitmap_rain_small_height, GxEPD_BLUE);
      } else if (strcmp(icon, "10d") == 0 || strcmp(icon, "10n") == 0){
        if (strcmp(icon, "10d") == 0){
          printSmallWeatherIcon(x, y - 7, "02d");
        } else if (strcmp(icon, "10n") == 0){
          printSmallWeatherIcon(x, y - 7, "02n");
        }
        display.drawBitmap(x + 20, y + 50, epd_bitmap_halfrain_small, epd_bitmap_halfrain_small_width, epd_bitmap_halfrain_small_height, GxEPD_BLUE);
      } else if (strcmp(icon, "11d") == 0 || strcmp(icon, "11n") == 0){
        display.drawBitmap(x + 26, y + 25, epd_bitmap_thunder_small, epd_bitmap_thunder_small_width, epd_bitmap_thunder_small_height, GxEPD_ORANGE);
        display.drawBitmap(x + 10, y + 2, epd_bitmap_cloud_small, epd_bitmap_cloud_small_width, epd_bitmap_cloud_small_height, GxEPD_BLACK);
      } else if (strcmp(icon, "13d") == 0 || strcmp(icon, "13n") == 0){
        display.drawBitmap(x + 10, y + 5, epd_bitmap_snow_small, epd_bitmap_snow_small_width, epd_bitmap_snow_small_height, GxEPD_BLUE);
      } else if (strcmp(icon, "50d") == 0 || strcmp(icon, "50n") == 0){
        display.drawBitmap(x + 10, y + 15, epd_bitmap_mist_small, epd_bitmap_mist_small_width, epd_bitmap_mist_small_height, GxEPD_BLACK);
      }
    }

  public:

    int yOffset;

    int refresh(int y, JsonObject weatherJsonObject){

      yOffset = y;

      _showFahrenheit = (bool)weatherJsonObject["showFahrenheit"];

      char openWeatherMapApiUrl[255];
      strcpy(openWeatherMapApiUrl, _openWeatherMapApiBaseUrl);
      strcat(openWeatherMapApiUrl, "&lat=");
      strcat(openWeatherMapApiUrl, (const char*)weatherJsonObject["location"]["lat"]);
      strcat(openWeatherMapApiUrl, "&lon=");
      strcat(openWeatherMapApiUrl, (const char*)weatherJsonObject["location"]["lon"]);

      client.connect(openWeatherMapApiUrl, 443);
      http.begin(client, openWeatherMapApiUrl);

      int openWeatherMapApiHttpResponseCode = http.GET();
      String openWeatherMapApiResponse = http.getString();

      Serial.println("openWeatherMapApiHttpResponseCode: ");
      Serial.println(openWeatherMapApiHttpResponseCode);
      Serial.println("openWeatherMapApi Response: ");
      Serial.println(openWeatherMapApiResponse);
      
      if(openWeatherMapApiHttpResponseCode != 200 && _initialized != true){
        return -1;
      } else if (openWeatherMapApiHttpResponseCode == 200) {
        deserializeJson(_openWeatherMapApiJsonObject, openWeatherMapApiResponse);
        setTime((long)_openWeatherMapApiJsonObject["current"]["dt"]);
        localTimezone.refresh((long)_openWeatherMapApiJsonObject["current"]["dt"], (const char*)_openWeatherMapApiJsonObject["timezone"], (int)_openWeatherMapApiJsonObject["timezone_offset"]);
        _initialized = true;
      }
      return 0;
    }

    void print(){
      printWeatherIcon(5, 5, (const char*)_openWeatherMapApiJsonObject["current"]["weather"][0]["icon"]);

      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols40pt);
      u8g2_for_adafruit_gfx.setFontMode(1);

      u8g2_for_adafruit_gfx.setForegroundColor(GxEPD_BLACK);

      u8g2_for_adafruit_gfx.setCursor(125, 50);
      if (_showFahrenheit){
        u8g2_for_adafruit_gfx.printf("%.0f℃ | %.0f℉", round((float)_openWeatherMapApiJsonObject["current"]["temp"]), round(celsius2fahrenheit((float)_openWeatherMapApiJsonObject["current"]["temp"])));
      } else {
        u8g2_for_adafruit_gfx.printf("%.0f℃", round((float)_openWeatherMapApiJsonObject["current"]["temp"]));
      }
      
      u8g2_for_adafruit_gfx.setFont(FreeSansBoldNotoSansWeatherSymbols24pt);
      u8g2_for_adafruit_gfx.setFontMode(1);
      
      u8g2_for_adafruit_gfx.setCursor(125, 90);
      u8g2_for_adafruit_gfx.printf("Feels like %.0f℃", round((float)_openWeatherMapApiJsonObject["current"]["feels_like"]));
      
      u8g2_for_adafruit_gfx.setCursor(125, 135);
      u8g2_for_adafruit_gfx.printf("☂%.0f%% UVI %d", + round((float)_openWeatherMapApiJsonObject["hourly"][1]["pop"] * 100), (int)_openWeatherMapApiJsonObject["current"]["uvi"]);
      
      for (int i = 2; i <= 10; i += 2){
        printHourlyWeather((i - 2) / 2 * (display.width() - DISPLAY_WIDTH_BORDER_OFFSET) / 5 , 155, _openWeatherMapApiJsonObject["hourly"][i]);
      }
    }

};
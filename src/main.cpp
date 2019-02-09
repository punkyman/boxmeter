#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "constant_data.h"

LiquidCrystal lcd(22,23,5,18,19,21);

void LCDPrint(const char* line1, const char* line2)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(line1);
    lcd.setCursor(0,1); 
    lcd.print(line2);
}

void setup()
{
    lcd.begin(16, 2);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_KEY);

    LCDPrint("  Connecting to ","     Wifi...    ");

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }

    LCDPrint("  Success, IP : ", WiFi.localIP().toString().c_str());
    delay(2000);
}

void loop()
{
    HTTPClient http;

    http.begin(HTTP_BOX_URL);
    int httpCode = http.GET();

    if(httpCode > 0)
    {
        WiFiClient* stream = http.getStreamPtr();
        String downloadData("D : ");
        String uploadData("U : ");
        char unused[2];

        if(stream->find(HTML_DOWNLOAD_TAG))
        {
            stream->readBytes(unused, 2); // reads ">
            downloadData += stream->readStringUntil('<');
        }

        if(stream->find(HTML_UPLOAD_TAG))
        {
            stream->readBytes(unused, 2); // reads ">
            uploadData += stream->readStringUntil('<');
        }

        LCDPrint(downloadData.c_str(), uploadData.c_str());
    }
    else
    {
        LCDPrint("  HTTP error :  ", http.errorToString(httpCode).c_str());
    }

    delay(15000);
}

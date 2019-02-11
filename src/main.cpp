#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "constant_data.h"

LiquidCrystal lcd(22,23,5,18,19,21);
String token;

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
    Serial.begin(9600);
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

    LCDPrint("    Fetching    ", "    token...    ");
    
    HTTPClient http;
    http.begin(API_TOKEN_URL);
    int httpCode = http.GET();

    if(httpCode > 0)
    {
        WiFiClient* stream = http.getStreamPtr();
        char unused;
        if(stream->find(XML_SESSIONINFO_TAG))
        {
            stream->readBytes(&unused, 1);
            token = stream->readStringUntil('<');
        }
    }

    LCDPrint("    Fetching    ", "     info...    ");
}

void loop()
{
    HTTPClient http;

    http.begin(API_STATISTICS_URL);
    http.addHeader("Cookie", token.c_str());
    int httpCode = http.GET();

    if(httpCode > 0)
    {
        Serial.write("OK");

        WiFiClient* stream = http.getStreamPtr();
        String longData;
        long downloadbytes = 0;
        long uploadbytes = 0;
        char unused;

        if(stream->find(XML_UPLOAD_TAG))
        {
            stream->readBytes(&unused, 1); // reads >
            longData = stream->readStringUntil('<');
            uploadbytes = longData.toInt();
        }

        if(stream->find(XML_DOWNLOAD_TAG))
        {
            stream->readBytes(&unused, 1); // reads >
            longData = stream->readStringUntil('<');
            downloadbytes = longData.toInt();
        }
        
        char downloadData[17];
        char uploadData[17];

        if(downloadbytes > GO_AS_BYTES)
            sprintf(downloadData, "D : %.2f Go", downloadbytes / BYTES_TO_GO);        
        else
            sprintf(downloadData, "D : %.2f Mo", downloadbytes / BYTES_TO_MO);

        if(uploadbytes > GO_AS_BYTES)
            sprintf(uploadData, "U : %.2f Go", uploadbytes / BYTES_TO_GO);
        else
            sprintf(uploadData, "U : %.2f Mo", uploadbytes / BYTES_TO_MO);

        LCDPrint(downloadData, uploadData);
    }
    else
    {
        LCDPrint("  HTTP error :  ", http.errorToString(httpCode).c_str());
    }

    delay(5000);
}

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Alarm.h>

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

void IRAM_ATTR handleButtonInterrupt() {
    stopAlarm();
}

void setup()
{
    pinMode(KEY_BUILTIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(KEY_BUILTIN), handleButtonInterrupt, FALLING);

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
}

void loop()
{
    HTTPClient http;

    if(token.length() == 0)
    {
        LCDPrint("    Fetching    ", "    token...    ");
        http.begin(API_TOKEN_URL);
    }
    else
    {
        http.begin(API_STATISTICS_URL);
        http.addHeader("Cookie", token.c_str());
    }

    int httpCode = http.GET();

    if(httpCode > 0)
    {
        WiFiClient* stream = http.getStreamPtr();

        if(token.length() == 0)
        {
            if(stream->find(XML_SESSIONINFO_TAG))
            {
                stream->read(); // removes '>'
                token = stream->readStringUntil('<');
            }
        }
        else
        {    
            String longData;
            unsigned long long downloadbytes = 0;
            unsigned long long uploadbytes = 0;
            char* end;

            if(stream->find(XML_UPLOAD_TAG))
            {
                stream->read(); // removes '>'
                longData = stream->readStringUntil('<');
                uploadbytes = strtoull(longData.c_str(), &end, 10);
            }

            if(stream->find(XML_DOWNLOAD_TAG))
            {
                stream->read(); // removes '>'
                longData = stream->readStringUntil('<');
                downloadbytes = strtoull(longData.c_str(), &end, 10);
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

            if(downloadbytes > BANDWIDTH_LIMIT_IN_BYTES)
                ringAlarm();
        }
    }
    else
    {
        LCDPrint("  HTTP error :  ", http.errorToString(httpCode).c_str());
        token = "";
    }

    delay(10000);
}

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Tone32.h>

#include "constant_data.h"

LiquidCrystal lcd(22,23,5,18,19,21);
String token;
pthread_t thread = 0;
bool ring = true;

void LCDPrint(const char* line1, const char* line2)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(line1);
    lcd.setCursor(0,1); 
    lcd.print(line2);
}

void* alarmThread(void* param)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while(1)
    {
        tone(17, NOTE_A4, 1000);
        tone(17, NOTE_A5, 1000);
    }
}

void IRAM_ATTR handleButtonInterrupt() {
    if(thread != 0)
    {
        pthread_cancel(thread);
        thread = 0;
    }
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
            long downloadbytes = 0;
            long uploadbytes = 0;

            if(stream->find(XML_UPLOAD_TAG))
            {
                stream->read(); // removes '>'
                longData = stream->readStringUntil('<');
                uploadbytes = longData.toInt();
            }

            if(stream->find(XML_DOWNLOAD_TAG))
            {
                stream->read(); // removes '>'
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

            if(downloadbytes > BANDWIDTH_LIMIT_IN_BYTES)
                pthread_create(&thread, NULL, alarmThread, NULL);
        }
    }
    else
    {
        LCDPrint("  HTTP error :  ", http.errorToString(httpCode).c_str());
        token = "";
    }

    delay(10000);
}

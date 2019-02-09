#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define USE_SERIAL Serial

const char* WIFI_SSID = "";
const char* WIFI_KEY = "";

LiquidCrystal lcd(22,23,5,18,19,21);

void setup()
{
    lcd.begin(16, 2);
    lcd.clear();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_KEY);

    lcd.setCursor(0,0);
    lcd.print("  Connecting to ");
    lcd.setCursor(0,1); 
    lcd.print("     Wifi...    ");

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  Success, IP : ");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    delay(2000);

}

void loop()
{
}

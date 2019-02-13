#include "Alarm.h"
#include <Arduino.h>
#include <Tone32.h>

pthread_t thread = 0;
bool stopped = false;

void* alarmThread(void* param)
{
    while(1)
    {
        if(stopped)
            return NULL;

        tone(17, NOTE_A4, 1000);
        tone(17, NOTE_A5, 1000);
    }
}

void ringAlarm()
{
    if(!stopped)
        pthread_create(&thread, NULL, alarmThread, NULL);
}

void stopAlarm()
{
    stopped = true;
}

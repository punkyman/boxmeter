#include "Alarm.h"
#include <Arduino.h>
#include <Tone32.h>

pthread_t thread = 0;

void* alarmThread(void* param)
{
    //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while(1)
    {
        tone(17, NOTE_A4, 1000);
        tone(17, NOTE_A5, 1000);
    }
}

void startAlarm()
{
    pthread_create(&thread, NULL, alarmThread, NULL);
}

void stopAlarm()
{
    if(thread != 0)
    {
        pthread_cancel(thread);
        thread = 0;
    }
}

bool isAlarmRunning()
{
    return thread != 0;
}

#include <Arduino.h>
#include "task3.h"
#include "events.h"

static bool compareKeys(BUTTONS *pSecret, BUTTONS *pKey)
{
    bool correct = true;
    for (uint8_t i = 0; i < 7; i++)
    {
        if (pSecret[i] != pKey[i]){
            correct = false;
            break;
        }
    }

    return correct;
}

void task3()
{

    enum class TaskStates
    {
        INIT,
        WAIT_CONFIG,
        COUNTING
    };
    static TaskStates taskState = TaskStates::INIT;
    const uint8_t ledBombCounting = 26;
    const uint8_t ledBombBoom = 25;
    const uint32_t BOMBINTERVAL = 1000U;
    const uint32_t LEDCOUNTERINTERVAL = 500U;

    static uint8_t bombCounter;
    static BUTTONS secret[7] = {BUTTONS::UP_BTN, BUTTONS::DOWN_BTN,
                                BUTTONS::UP_BTN, BUTTONS::DOWN_BTN,
                                BUTTONS::UP_BTN, BUTTONS::UP_BTN,
                                BUTTONS::ARM_BTN};

    static BUTTONS disarmKey[7] = {BUTTONS::NONE};

    static uint8_t ledBombCountingState;
    static uint32_t initBombTimer;
    static uint32_t initLedCounterTimer;
    static uint8_t keyCounter;

    switch (taskState)
    {
    case TaskStates::INIT:
    {

        pinMode(ledBombCounting, OUTPUT);
        pinMode(ledBombBoom, OUTPUT);
        digitalWrite(ledBombBoom, LOW);
        ledBombCountingState = HIGH;
        digitalWrite(ledBombCounting, ledBombCountingState);
        bombCounter = 20;
        keyCounter = 0;
        taskState = TaskStates::WAIT_CONFIG;
        break;
    }
    case TaskStates::WAIT_CONFIG:
    {

        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            if (buttonEvt.whichButton == BUTTONS::UP_BTN)
            {
                if (bombCounter < 60)
                    bombCounter++;
            }
            else if (buttonEvt.whichButton == BUTTONS::DOWN_BTN)
            {
                if (bombCounter > 10)
                    bombCounter--;
            }
            else if (buttonEvt.whichButton == BUTTONS::ARM_BTN)
            {
                initLedCounterTimer = millis();
                initBombTimer = millis();
                keyCounter = 0;
                taskState = TaskStates::COUNTING;
            }
            Serial.print("Counter: ");
            Serial.print(bombCounter);
            Serial.print("\n");
        }

        break;
    }
    case TaskStates::COUNTING:
    {

        uint32_t timeNow = millis();

        if ((timeNow - initBombTimer) > BOMBINTERVAL)
        {
            initBombTimer = timeNow;
            bombCounter--;
            Serial.print("Counter: ");
            Serial.print(bombCounter);
            Serial.print("\n");
            if (bombCounter == 0)
            {
                ledBombCountingState = HIGH;
                Serial.print("BOMB BOOM\n");
                digitalWrite(ledBombBoom, HIGH);
                delay(2000);
                digitalWrite(ledBombBoom, LOW);
                digitalWrite(ledBombCounting, ledBombCountingState);
                bombCounter = 20;
                taskState = TaskStates::WAIT_CONFIG;
            }
        }
        if ((timeNow - initLedCounterTimer) > LEDCOUNTERINTERVAL)
        {
            initLedCounterTimer = timeNow;
            ledBombCountingState = !ledBombCountingState;
            digitalWrite(ledBombCounting, ledBombCountingState);
        }

        if (buttonEvt.trigger == true)
        {
            buttonEvt.trigger = false;
            disarmKey[keyCounter] = buttonEvt.whichButton;
            keyCounter++;
            if (keyCounter == 7)
            {
                keyCounter = 0;
                if (compareKeys(secret, disarmKey) == true)
                {
                    ledBombCountingState = HIGH;
                    digitalWrite(ledBombCounting, ledBombCountingState);
                    Serial.print("BOMB DISARM\n");
                    bombCounter = 20;
                    taskState = TaskStates::WAIT_CONFIG;
                }
            }
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

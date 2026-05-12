// fsm.c
#include "fsm.h"
#include "driverlib.h"
#include "device.h"
#include <stdbool.h>
#include <stdlib.h>

volatile ConverterState_t g_converterState = CONVERTER_STATE_IDLE;

volatile bool g_enableModulation = false;

static int filteredValue = 500;

static int adcValue = -500;

static int adcStep = 50;

static int filteredHistory[100];

static int rawHistory[100];

static int historyIndex = 0;

static int pwmCounter = 0;

static int dutyCycle = 0;

#define FILTER_SIZE 16

static int adcBuffer[FILTER_SIZE];

static int bufferIndex = 0;

static int readSimulatedADC(void)
{
    adcValue += adcStep;

    if(adcValue >= 500)
    {
        adcStep = -50;
    }

    if(adcValue <= -500)
    {
        adcStep = 50;
    }

    int noise = (rand() % 101) - 50;
    return adcValue + noise;
}

static int movingAverageFilter(int newSample)
{
    int sum = 0;

    adcBuffer[bufferIndex] = newSample;

    bufferIndex++;

    if(bufferIndex >= FILTER_SIZE)
    {
        bufferIndex = 0;
    }

    for(int i = 0; i < FILTER_SIZE; i++)
    {
        sum += adcBuffer[i];
    }

    return sum / FILTER_SIZE;
}

void FSM_Init(void)
{

    pwmCounter++;

if(pwmCounter >= 100)
{
    pwmCounter = 0;
}
    g_converterState = CONVERTER_STATE_IDLE;

    g_enableModulation = false;


    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);

    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_OUT);


    GPIO_writePin(31,1);
    GPIO_writePin(34,1);
}


void FSM_RunCycle(void)
{
    int rawValue = readSimulatedADC();

    filteredValue = movingAverageFilter(rawValue);

    dutyCycle = abs(filteredValue) / 5;

    if(dutyCycle > 100)
{
    dutyCycle = 100;
}

    rawHistory[historyIndex] = rawValue;
    
    filteredHistory[historyIndex] = filteredValue;

    historyIndex++;

if(historyIndex >= 100)
{
    historyIndex = 0;
}

    if(g_enableModulation == false)
    {
        g_converterState = CONVERTER_STATE_IDLE;
    }
    else
    {
        if(filteredValue > 0)
        {
            g_converterState = CONVERTER_STATE_POSITIVE;
        }
        else
        {
            g_converterState = CONVERTER_STATE_NEGATIVE;
        }
    }


    switch(g_converterState)
    {
        case CONVERTER_STATE_IDLE:

            GPIO_writePin(31,1);
            GPIO_writePin(34,1);

            break;


        case CONVERTER_STATE_POSITIVE:

        if(pwmCounter < dutyCycle)
        {
            GPIO_writePin(31,0);
        }
        else
        {
            GPIO_writePin(31,1);
        }

        GPIO_writePin(34,1);

            break;


        case CONVERTER_STATE_NEGATIVE:

        GPIO_writePin(31,1);

        if(pwmCounter < dutyCycle)
        {
            GPIO_writePin(34,0);
        }
        else
        {
            GPIO_writePin(34,1);
        }

            break;


        default:

            break;
    }
}

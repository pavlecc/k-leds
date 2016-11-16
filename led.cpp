#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "led.h"
#include "input.h"

LED::LED(char* _devName)
{
    devName = _devName;
    file = open(devName, O_RDWR);
    if (file < 0)
    {
        printf("\n\n**ERROR** LED::Could not open the file \"%s\"\n\n", devName);
    }
    close(file);
}

LED::~LED() {}

void LED::Set(int _channel, float _value)
{
    char buffer[16];
    snprintf(buffer, 16, "%d=%.2f\n", _channel, _value);
    file = open(devName, O_RDWR);
    int bufLen = strlen(buffer);
    if (!(file < 0))
    {
        write(file, buffer, bufLen);
        close(file);
    }
}

// https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72 
void HueToRGB(float _h, float _s, float _v, float& _r, float& _g, float& _b)
{
    float fHPrime = _h / 60.0f;
    float fC = _v * _s;
    float fX = fC * (1.0f - fabs(fmod(fHPrime, 2.0f) - 1.0f));
    float fM = _v - fC;
    int iHPrime = floor(fmod(fHPrime, 6.0f));

    switch (iHPrime)
    {
    case 0: _r = fC; _g = fX; _b = 0.0f; break;
    case 1: _r = fX; _g = fC; _b = 0.0f; break;
    case 2: _r = 0.0f; _g = fC; _b = fX; break; 
    case 3: _r = 0.0f; _g = fX; _b = fC; break;
    case 4: _r = fX; _g = 0.0f; _b = fC; break;
    case 5: _r = fC; _g = 0.0f; _b = fX; break;
    default: _r = 0.0f; _g = 0.0f; _b = 0.0f; break;
    }

    _r += fM; _g += fM; _b += fM;
}

#define TIMEOUT   33
#define TIMEOUT_MULTIPLIER 4

void* LED::Run(void* _data)
{
    LED led((char*)_data);
    if (led.file < 0)
    {
        return NULL;
    }
    float red = 0.0f; float green = 0.0f; float blue = 0.0f;
    float dutyRatio = 0.0f;
    bool isTapping = false;
    bool isBlinking = false;
    bool ledsOn = false;
    unsigned long timeOutCnt = 0;
    while(1)
    {
        if (Input::GetInstance().IsJustPressed(EMidiCode_On, TIMEOUT_MULTIPLIER * TIMEOUT))
        {
            ledsOn = true;
        }
        else if (Input::GetInstance().IsJustPressed(EMidiCode_Off, TIMEOUT_MULTIPLIER * TIMEOUT))
        {
            ledsOn = false;
        }
        else
        {
            unsigned long long tapStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Tap);
            unsigned long long speedStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Speed);
            unsigned long long onStamp = Input::GetInstance().GetTimeStamp(EMidiCode_On);
            unsigned long long offStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Off);

            isTapping =  tapStamp > speedStamp && tapStamp > onStamp && tapStamp > offStamp;
            isBlinking = speedStamp > tapStamp && speedStamp > onStamp && speedStamp > offStamp;

            if (isTapping)
            {
                ledsOn = Input::GetInstance().IsJustPressed(EMidiCode_Tap, TIMEOUT_MULTIPLIER * TIMEOUT) ||
                         Input::GetInstance().IsTapRepeated(EMidiCode_Tap, TIMEOUT_MULTIPLIER * TIMEOUT);
            }
            else if (isBlinking)
            {
                dutyRatio = Input::GetInstance().GetValue(EMidiCode_Speed);
                ledsOn = timeOutCnt % (2 * TIMEOUT_MULTIPLIER + lround((TIMEOUT_MULTIPLIER * 6) * (1.0f - dutyRatio))) < TIMEOUT_MULTIPLIER;
            }
        }
        
        if (ledsOn)
        {
            float hue = Input::GetInstance().GetValue(EMidiCode_Hue);
            float sat = Input::GetInstance().GetValue(EMidiCode_Sat);
            float val = Input::GetInstance().GetValue(EMidiCode_Val);
            HueToRGB(hue * 360.0f, sat, val, red, green, blue);
        }
        else
        {
            red = 0.0f; green = 0.0f; blue = 0.0f;
        }
        
        led.Set(27, red);
        led.Set(17, green);
        led.Set(22, blue); 
        
        timeOutCnt++;
        usleep(TIMEOUT * 1000);
    }
}


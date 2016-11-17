#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "led.h"
#include "input.h"
#include "constants.h"
#include "util.h"

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

void LED::Set(int _pinR, int _pinG, int _pinB, Vec3 _color)
{
    char buffer[16];
    int bufLen = 0;
    file = open(devName, O_RDWR);    
    if (!(file < 0))
    {
        snprintf(buffer, 16, "%d=%.2f\n", _pinR, _color.r);
        bufLen = strlen(buffer);
        write(file, buffer, bufLen);
        snprintf(buffer, 16, "%d=%.2f\n", _pinG, _color.g);
        bufLen = strlen(buffer);
        write(file, buffer, bufLen);
        snprintf(buffer, 16, "%d=%.2f\n", _pinB, _color.b);
        bufLen = strlen(buffer);
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

void LED::DoPattern(unsigned char _pattern, Vec3 _color, unsigned long _patternCnt)
{
    switch (_pattern)
    {
    default:
    case 0:
        Set(EGpioPins_1R, EGpioPins_1G, EGpioPins_1B, _color);
        Set(EGpioPins_2R, EGpioPins_2G, EGpioPins_2B, _color);
        break;
    case 1:
        unsigned long temp = _patternCnt % 2;
        Set(EGpioPins_1R, EGpioPins_1G, EGpioPins_1B, temp == 0 ? _color : Vec3::GetZero());
        Set(EGpioPins_2R, EGpioPins_2G, EGpioPins_2B, temp == 1 ? _color : Vec3::GetZero());
        break;
    }
}

void* LED::Run(void* _data)
{
    LED led((char*)_data);
    if (led.file < 0)
    {
        return NULL;
    }
    
    Vec3 rgbColor = Vec3::GetZero();
    float hue = 0.0f; float sat = 0.0f; float val = 0.0f;
    float dutyRatio = 0.0f;
    bool isTapping = false;
    bool isBlinking = false;
    bool ledsOn = false;
    bool ledsOn_prev = false;
    unsigned char pattern = 0;
    unsigned long patternCnt = 0;
    unsigned long timeOutCnt = 0;

    while(1)
    {
        ledsOn_prev = ledsOn;

        if (Input::GetInstance().IsJustPressed(EMidiCode_On, K_LedTimeoutMultiplier * K_LedTimeoutMs))
        {
            ledsOn = true;
            pattern = 0;
        }
        else if (Input::GetInstance().IsJustPressed(EMidiCode_Off, K_LedTimeoutMultiplier * K_LedTimeoutMs))
        {
            ledsOn = false;
            pattern = 0;
        }
        else
        {
            if (Input::GetInstance().IsJustPressed(EMidiCode_Pattern1, K_LedTimeoutMultiplier * K_LedTimeoutMs))
            {
                pattern = 1;
                patternCnt = 0;
            }
            if (Input::GetInstance().IsJustPressed(EMidiCode_Tap, K_LedTimeoutMultiplier * K_LedTimeoutMs) ||
                Input::GetInstance().IsJustPressed(EMidiCode_Speed, K_LedTimeoutMultiplier * K_LedTimeoutMs))
            {
                pattern = 0;
            }

            unsigned long long tapStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Tap);
            unsigned long long speedStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Speed);
            unsigned long long onStamp = Input::GetInstance().GetTimeStamp(EMidiCode_On);
            unsigned long long offStamp = Input::GetInstance().GetTimeStamp(EMidiCode_Off);

            isTapping =  tapStamp > speedStamp && ((tapStamp > onStamp && tapStamp > offStamp) || pattern > 0);
            isBlinking = speedStamp > tapStamp && ((speedStamp > onStamp && speedStamp > offStamp) || pattern > 0);

            if (isTapping)
            {
                ledsOn = Input::GetInstance().IsJustPressed(EMidiCode_Tap, K_LedTimeoutMultiplier * K_LedTimeoutMs) ||
                         Input::GetInstance().IsTapRepeated(EMidiCode_Tap, K_LedTimeoutMultiplier * K_LedTimeoutMs);
            }
            else if (isBlinking)
            {
                dutyRatio = Input::GetInstance().GetValue(EMidiCode_Speed);
                ledsOn = timeOutCnt % (2 * K_LedTimeoutMultiplier + lround((K_LedTimeoutMultiplier * 6) * (1.0f - dutyRatio))) < K_LedTimeoutMultiplier;
            }
        }

        if (ledsOn)
        {
            if (pattern > 0 && ledsOn && !ledsOn_prev)
            {
                patternCnt++;
            }
            hue = Input::GetInstance().GetValue(EMidiCode_Hue);
            sat = Input::GetInstance().GetValue(EMidiCode_Sat);
            val = Input::GetInstance().GetValue(EMidiCode_Val);
            HueToRGB(hue * 360.0f, sat, val, rgbColor.r, rgbColor.g, rgbColor.b);
        }
        else
        {
            rgbColor = Vec3::GetZero();
        }
        
        led.DoPattern(pattern, rgbColor, patternCnt);
        
        timeOutCnt++;
        usleep(K_LedTimeoutUs);
    }
}


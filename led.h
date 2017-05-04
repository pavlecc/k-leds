#pragma once

#include "constants.h"
#include "util.h"

class LED
{
private:
    char* devName;
    static char devNameDefault[20];
    int file;
    bool initialized;
    Vec3 colors[K_StripeCount];

public:
    LED(char* _devName);
    ~LED();
    void Set(int _stripe, int _pinR, int _pinG, int _pinB, Vec3 _color);
    void DoPattern(unsigned char _pattern, Vec3 _color, unsigned long _timeCnt);
    static void* Run(void* _data);
};


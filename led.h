#pragma once

#include "util.h"

class LED
{
private:
    char* devName;
    static char devNameDefault[20];
    int file;

public:
    LED(char* _devName);
    ~LED();
    void Set(int _pinR, int _pinG, int _pinB, Vec3 _color);
    void DoPattern(unsigned char _pattern, Vec3 _color, unsigned long _timeCnt);
    static void* Run(void* _data);
};


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
    static void* Run(void* _data);
};


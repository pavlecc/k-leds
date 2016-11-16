#pragma once

class LED
{
private:
    char* devName;
    static char devNameDefault[20];
    int file;
public:
    LED(char* _devName);
    ~LED();
    void Set(int _channel, float _value);
    static void* Run(void* _data);
};


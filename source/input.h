#pragma once

#include "constants.h"

struct MidiCtrl
{
    EMidiCode          code;
    unsigned char      value;
    bool               pressed;
    unsigned long long timeStamp;
    unsigned long      tapInterval;
    unsigned long long tapStamps[K_MaxTapCount];
    unsigned char      tapCount;
};

class Input
{
public:
    static Input& GetInstance();
    static void* Run(void* _data);
    static bool s_InputActive;

    MidiCtrl* GetMidiCtrlByCode(EMidiCode _code);
    bool IsJustPressed(EMidiCode _code, unsigned long _time);
    bool IsTapRepeated(EMidiCode _code, unsigned long _time);
    bool IsPressed(EMidiCode _code);
    float GetValue(EMidiCode _code);
    unsigned long long GetTimeStamp(EMidiCode _code);

private:
    Input();
    Input(Input const&);
    void operator=(Input const&);
    void Read();

    static Input* instance;
    MidiCtrl m_Controls[K_MidiNumControls];
};


#include <unistd.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "input.h"
#include "util.h"

Input::Input()
{
    // Initialize all to 0
    for (int i = 0; i < K_MidiNumControls; i++)
    {
        memset(&m_Controls[i], 0, sizeof(MidiCtrl));
    }

    // Initialize midi map
    m_Controls[0].code = EMidiCode_Hue;
    m_Controls[1].code = EMidiCode_Sat;
    m_Controls[2].code = EMidiCode_Val;
    m_Controls[3].code = EMidiCode_Speed;
    m_Controls[4].code = EMidiCode_Tap;
    m_Controls[5].code = EMidiCode_Pattern1;
    m_Controls[6].code = EMidiCode_On;
    m_Controls[7].code = EMidiCode_Off;
    m_Controls[8].code = EMidiCode_None;
}

Input& Input::GetInstance()
{
    static Input instance;
    return instance;
}

bool Input::IsJustPressed(EMidiCode _code, unsigned long _milis)
{
    MidiCtrl* ctrl = GetMidiCtrlByCode(_code);
    if (ctrl)
    {
        long interval = (Util::GetCurrentMilis() - ctrl->timeStamp);
        return  interval < _milis;
    }
    return false;
}

bool Input::IsTapRepeated(EMidiCode _code, unsigned long _milis)
{
    MidiCtrl* ctrl = GetMidiCtrlByCode(_code);
    if (ctrl && ctrl->tapInterval > 0)
    {
        long interval = (Util::GetCurrentMilis() - ctrl->timeStamp);
        if (interval < ctrl->tapInterval)
        {
            return false;
        }
        return (interval % ctrl->tapInterval) < _milis;
    }
    return false;
}

bool Input::IsPressed(EMidiCode _code)
{
    return GetMidiCtrlByCode(_code)->pressed;
}

float Input::GetValue(EMidiCode _code)
{
    return GetMidiCtrlByCode(_code)->value / K_MidiMaxCC;
}

unsigned long long Input::GetTimeStamp(EMidiCode _code)
{
    return GetMidiCtrlByCode(_code)->timeStamp;
}

MidiCtrl* Input::GetMidiCtrlByCode(EMidiCode _code)
{
    for (int i = 0; i < K_MidiNumControls; i++)
    {
        if (m_Controls[i].code == _code)
        {
            return &m_Controls[i];
        }
    }
    return NULL;
}

//////////////////////////////
//
// Input::Run -- Thread function which waits around until a MIDI 
//      input byte arrives and then it prints the byte to the terminal.
//      This thread function does not end gracefully when the program
//      stops.

void* Input::Run(void* _data)
{
    snd_rawmidi_t* midiIn = (snd_rawmidi_t*)_data;
    char buffer[3];
    MidiCtrl* ctrl;
    unsigned long long currentMilis;

    while (1) {
        if (midiIn == NULL) {
            break;
        }
        if (snd_rawmidi_read(midiIn, buffer, 3) < 0) {
            printf("\n**ERROR** Problem reading MIDI input\n\n");
            break;
        }

        ctrl = NULL;
        currentMilis = Util::GetCurrentMilis();

        switch (buffer[0])
        {
        case 0x90:    
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[1]);
                if (ctrl)
                {
                    if (currentMilis > (ctrl->timeStamp + K_MaxTapInterval))
                    {
                        ctrl->tapCount = 1;
                        memset(ctrl->tapStamps, 0, K_MaxTapCount * sizeof(long));
                    }
                    else
                    {
                        ctrl->tapCount = ctrl->tapCount + 1 < K_MaxTapCount ? ctrl->tapCount + 1 : K_MaxTapCount;
                    }
                    for (int ri = ctrl->tapCount - 1; ri > 0; ri--)
                    {
                        ctrl->tapStamps[ri] = ctrl->tapStamps[ri - 1];
                    }
                    ctrl->tapStamps[0] = currentMilis;
                    ctrl->tapInterval = 0;
                    if (ctrl->tapCount > 1)
                    {
                        for (int ti = 0; ti < ctrl->tapCount - 1; ti++)
                        {
                            ctrl->tapInterval += (ctrl->tapStamps[ti] - ctrl->tapStamps[ti + 1]);
                        }
                        ctrl->tapInterval = lround(ctrl->tapInterval / (float)(ctrl->tapCount - 1));
                    }

                    ctrl->timeStamp = currentMilis;
                    ctrl->value = buffer[2];
                    ctrl->pressed = true;
                }
            }
            break;
        case 0x80:
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[1]);
                if (ctrl)
                {
                    ctrl->pressed = false;
                }
            }
            break;
        case 0xB0:
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[1]);
                if (ctrl)
                {
                    ctrl->timeStamp = currentMilis;
                    ctrl->value = buffer[2];
                }
            }
            break;
        default:
            break;
        }
        fflush(stdout);
        usleep(K_InputTimeoutUs);
    }

    return NULL;
}

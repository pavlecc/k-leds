#include <unistd.h>
#include <math.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "input.h"
#include "util.h"

bool Input::s_InputActive = false;

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
    m_Controls[5].code = EMidiCode_On;
    m_Controls[6].code = EMidiCode_Off;
    m_Controls[7].code = EMidiCode_Pattern1;
    m_Controls[8].code = EMidiCode_Pattern2;
    m_Controls[9].code = EMidiCode_Pattern3;
    m_Controls[10].code = EMidiCode_Pattern4;
    m_Controls[11].code = EMidiCode_Pattern5;
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
    snd_rawmidi_t* midiIn = NULL;
    char* portName = (char*)_data;
    char buffer[K_MidiBufferSize];
    MidiCtrl* ctrl;
    unsigned long long currentMilis;
    int messageIdx = 0;

    while (1) {

        if (midiIn == NULL)
        {
            if (snd_rawmidi_open(&midiIn, NULL, portName, 0) < 0)
            {
                s_InputActive = false;
                usleep(K_InputTimeoutUs);
                continue;
            }
            s_InputActive = true;
            log_info("Midi device connected\n");
        }

        if (snd_rawmidi_read(midiIn, buffer, K_MidiBufferSize) < 0) {
            log_error("\n**ERROR** Problem reading MIDI input\n\n");
            midiIn = NULL;
            s_InputActive = false;
            usleep(K_InputTimeoutUs);
            continue;
        }

        ctrl = NULL;
        currentMilis = Util::GetCurrentMilis();

        for (messageIdx = 3; messageIdx < K_MidiBufferSize; messageIdx += 3)
        {
            if (buffer[messageIdx] == 0)
            {
                break;
            }
        }
        messageIdx -= 3;

        log_info("Midi message: %x %x %x\n", buffer[messageIdx], buffer[messageIdx + 1], buffer[messageIdx + 2]);

        switch (buffer[messageIdx])
        {
        case 0x90:    
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[messageIdx + 1]);
                if (ctrl)
                {
                    if (currentMilis > (ctrl->timeStamp + K_MaxTapInterval))
                    {
                        ctrl->tapCount = 1;
                        memset(ctrl->tapStamps, 0, K_MaxTapCount * sizeof(long));
                    }
                    else
                    {
                        ctrl->tapCount++;
                        ctrl->tapCount = ctrl->tapCount < K_MaxTapCount ? ctrl->tapCount : K_MaxTapCount;
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
                        float fTapCount = (float)(ctrl->tapCount - 1);
                        if (fTapCount < 0.5) fTapCount = 1.0f;
                        ctrl->tapInterval = lround(ctrl->tapInterval / fTapCount);
                        log_info("Tap interval: %lu\n", ctrl->tapInterval);
                    }

                    ctrl->timeStamp = currentMilis;
                    ctrl->value = buffer[messageIdx + 2];
                    ctrl->pressed = true;
                }
            }
            break;
        case 0x80:
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[messageIdx + 1]);
                if (ctrl)
                {
                    ctrl->pressed = false;
                }
            }
            break;
        case 0xB0:
            {
                ctrl = GetInstance().GetMidiCtrlByCode((EMidiCode)buffer[messageIdx + 1]);
                if (ctrl)
                {
                    ctrl->timeStamp = currentMilis;
                    ctrl->value = buffer[messageIdx + 2];
	        }
            }
            break;
        default:
            break;
        }
        fflush(stdout);
        memset(buffer, 0, K_MidiBufferSize);
        usleep(K_InputTimeoutUs);
    }

    return NULL;
}

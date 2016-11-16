#pragma once

#define K_MidiNumControls 16
#define K_MidiMaxCC       128.0f

enum EMidiCode
{
	EMidiCode_None = 0,
    EMidiCode_Speed = 3,
    EMidiCode_Val = 4,
    EMidiCode_Hue = 5,
    EMidiCode_Sat = 6,
    EMidiCode_On = 45,
    EMidiCode_Off = 46,
    EMidiCode_Tap = 47,
    EMidiCode_Pattern
};

#pragma once

#include "midi_mapping.h"
#include "gpio_mapping.h"

#define K_StripeCount           6

#define K_MaxTapCount           8
#define K_MaxTapInterval        2000

#define K_MidiNumControls       16
#define K_MidiMaxCC             128.0f
#define K_MidiBufferSize        15

#define K_LedTimeoutMs          33
#define K_LedTimeoutUs          33000
#define K_LedTimeoutMultiplier  4

#define K_InputTimeoutUs        10000

#define K_PiBlasterDev          "/dev/pi-blaster"
#define K_MidiDev               "hw:1,0,0"

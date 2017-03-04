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
#define K_PiBlasterInit         "/home/pi/pi-blaster/pi-blaster --gpio \
								 2,3,4,5,7,8,9,10,11,13,16,17,19,20,21,22,25,27"

#ifdef DEBUG
#define log_error(...) fprintf(stderr, __VA_ARGS__)
#define log_info(...) fprintf(stdout, __VA_ARGS__)
#else
#define log_error(...) do {} while (false)
#define log_info(...) do {} while (false)
#endif
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "led.h"
#include "input.h"
#include "constants.h"

int main(int argc, char* argv[])
{
    pthread_t t1;
    pthread_t t2;

    // Parse input
    const char* fileName = K_PiBlasterDev;
    const char* portName = K_MidiDev;
    for (int i = 1; i < argc; i++)
    {
        if (strncmp("/dev/", argv[i], 4) == 0)
        {
            fileName = argv[i];
        }
        if (strncmp("hw:", argv[i], 3) == 0)
        {
            portName = argv[i];
        }
    }
    
    snd_rawmidi_t* midiIn;
    if (snd_rawmidi_open(&midiIn, NULL, portName, 0) < 0)
    {
        printf("\n**ERROR** Problem opening MIDI input: %s\n\n", portName);
        exit(1);
    }

    Input::GetInstance();

    pthread_create(&t1, NULL, LED::Run, (char*)fileName);
    pthread_create(&t2, NULL, Input::Run, midiIn);
    pthread_join(t2, NULL);
 
    return 0;
}


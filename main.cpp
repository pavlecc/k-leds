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
    // Start Pi-Blaster
    int piBlasterStatus = system(K_PiBlasterInit);
    if (piBlasterStatus != 0)
    {
        log_error("\n**ERROR** Problem starting Pi-Blaster");
        exit(1);
    }
    // Wait a bit for Pi-Blaster to activate
    usleep(K_InputTimeoutUs);

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
    
    // Initiate input
    Input::GetInstance();

    // Start threads - input and led
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, LED::Run, (char*)fileName);
    pthread_create(&t2, NULL, Input::Run, (char*)portName);
    pthread_join(t2, NULL);
 
    return 0;
}


#include <pthread.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "led.h"
#include "input.h"
#include "constants.h"

void intHandler(int dummy)
{
    system(K_PiBlasterKill);
    exit(0);
}

int main(int argc, char* argv[])
{
    // Register interrupt handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = intHandler;
    sigaction(SIGINT, &sa, NULL);

    // Start Pi-Blaster
    int piBlasterStatus = system(K_PiBlasterExec);
    if (piBlasterStatus != 0)
    {
        log_error("\n\n**ERROR** Problem starting Pi-Blaster\n\n");
        exit(1);
    }
    log_info("Starting pi-blaster..\n");

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

    intHandler(0);
 
    return 0;
}


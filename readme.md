# K-Leds
Custom LED solution with Raspberry Pi and MIDI input.

Developed for Kišobran parties in Belgrade, Serbia.

![Kišobran party](https://scontent.fbeg1-1.fna.fbcdn.net/v/t31.0-8/16179022_10154063483077470_8420772008124729658_o.jpg?oh=33f9a0ede82aa55c3bdb98c21f7da793&oe=5A7DEA30)

## Dependencies
[Pi-Blaster](https://github.com/sarfata/pi-blaster) @ [1035ad7dffb270c40eec1bb3a654171a755fba98](https://github.com/sarfata/pi-blaster/commit/1035ad7dffb270c40eec1bb3a654171a755fba98)

## Setup
* Install Pi-Blaster on your Raspberry Pi
* Change *K_PiBlasterExec* from *constants.h* to point to the right location
* Build K-Leds using build.sh
* Build with *-DDEBUG* for debugging info
##### MIDI Mapping
* Start debug version
* Attached MIDI controller and observe the K-Leds output
* Use this info to figure out the buttons codes
* Apply these codes in *midi_mapping.h*
##### GPIO Mapping
* The code works with 6 RGB LED stripes
* Use *gpio_mapping.h* for GPIO-LED connection or adapt the header file to your setup

## How's it done
Incoming..
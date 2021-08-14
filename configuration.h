#ifndef CONFIGURATION_H
#define CONFIGUARTION_H

#include "hardware/uart.h"

//#define DEBUG 



// PIANO SETTINGS

#define NUM_KEYS 88
#define FIRST_KEY 20 // where the first key is relative to the first midi note




// MIDI SETTINGS

#define BUFFER_SIZE 30
#define VISUALISER_CHANNEL 15

// LED SETTINGS

#define LED_PIN 11
#define LEDS_PER_KEY 2
#define NUM_LEDS NUM_KEYS * LEDS_PER_KEY
#define LED_FREQUENCY (uint16_t)90
#define LED_DITHERING 2 // By pulsing between 8 bit values, multiplies resolution by this amount. Beware: Dithering is limited by the max LED speed to (33333/NUM_LEDS)/LED_FREQUENCY and a frequency of ~100Hz is needed to stop visible flickering.
#define LED_STRIP_INVERT true // whether to flip the direction the LEDs are indexed in


#define CURRENT_PER_LED 60 // max (overestimate) per LED in mA to prevent overcurrent
#define MAX_TOTAL_CURRENT 2000 // max current for whole system









#endif
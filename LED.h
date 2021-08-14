#ifndef LED_H
#define LED_H
#include "configuration.h"


#include "WS2812.hpp"


#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/timer.h"

#include <list>
#include <cmath>







bool repeating_timer_callback (struct repeating_timer *t);

class LED {
    public:
        enum commandType {
            KEY_ON,
            KEY_OFF,
            EFFECT,
        };
        enum hsvChannelNumbers {
            H,S,V
        };

        enum modes {
            DUAL,LEFT,RIGHT,SINGLE
        };

    private:
        const uint8_t gamma8[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
        2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
        5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
        10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
        17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
        25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
        37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
        51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
        69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
        90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
        115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
        144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
        177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
        215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
    
        WS2812 * ledStrip;
        
        enum rgbChannelNumbers {
            R,G,B
        };
        
        struct RgbColour
        {
            uint16_t r;
            uint16_t g;
            uint16_t b;
        };

        struct HsvColour
        {
            uint16_t h;
            uint16_t s;
            uint16_t v;
        };
        
        struct repeating_timer timer;
        
        RgbColour HsvToRgb (HsvColour hsv);
        
        struct command {
            commandType type;
            uint8_t led=0;
            hsvChannelNumbers channel=H;
            int16_t target  =0;
            uint16_t duration =0;
            uint16_t delay   =0;
            bool finished = false;
            
        };
        
        // struct change {
        //     uint8_t led=0;
        //     hsvChannelNumbers channel=h;
        //     int16_t newCurrent  =0;
        //     int8_t changeNumber=-1;
        //     uint16_t remainingDuration=0;
        // };
        
        
        // HsvColour currentHSV [NUM_LEDS];
        // RgbColour currentRGB [NUM_LEDS];
        
        int16_t currentHSV [NUM_LEDS][3];
        int16_t currentRGB [NUM_LEDS][3];
        
        
        std::list<command> commandQueue;
        
        // int16_t currentHSV[NUM_LEDS][3]; // The current value of each RGB channel
        // int16_t targetHSV [NUM_LEDS][3]; // The target value of each RGB channel
        // int16_t rateHSV   [NUM_LEDS][3]; // The rate of change towards the target of each RGB channel
        // int16_t delayHSV [NUM_LEDS][3]; // The delay before the change happens
        
        uint8_t multiplier = 128;  // The max duration of a fade is limited to (channel range * (1000/LED_FREQUENCY))ms, so by storing the channel values across a greater range, interpolation in a fade is smoother and fades can last longer. For example, a range of 256 only gives a max fade of 4s at 60hz
        uint8_t ditheringLevel = LED_DITHERING;
        uint8_t ditheringCounter=0;
        uint16_t refreshDelay; // Theay delay between each update to the LEDs
        uint16_t counter=0;
        int16_t calcRate (command  currentCommand);
        
        void dither ();
        
        //uint64_t interruptStart;
    public:
        
        
        
        LED();
        
        void show();
        
        void onInterrupt();
        
        void set(uint8_t keyNum, int16_t targetVal, hsvChannelNumbers channel, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        void setHSV          (uint8_t keyNum, int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);  
        void changeHSV       (uint8_t keyNum, int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        void setHue        (uint8_t keyNum, uint8_t hue        , modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void setSaturation (uint8_t keyNum, uint8_t saturation , modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void setBrightness (uint8_t keyNum, uint8_t brightness , modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        // void changeHue        (uint8_t keyNum, int16_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        // void changeSaturation (uint8_t keyNum, int16_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void changeBrightness (uint8_t keyNum, int16_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        void setFill (uint8_t targetVal, hsvChannelNumbers channel, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        void setHSVFill    (int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void changeHSVFill (int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        void setHueFill        (uint8_t hue       , modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void setSaturationFill (uint8_t saturation, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        void setBrightnessFill (uint8_t brightness, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        // void changeHueFill        (uint8_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        // void changeSaturationFill (uint8_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        // void changeBrightnessFill (uint8_t change, modes mode, uint16_t duration=0, uint16_t delay=0, commandType type=EFFECT);
        
        uint8_t getHue(uint8_t keyNum);
        uint8_t getSaturation(uint8_t keyNum);
        uint8_t getBrightness(uint8_t keyNum);
        
        
};


extern LED * led;

#endif
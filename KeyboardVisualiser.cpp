//#include "Arduino.h"
#include "configuration.h"

#include "LED.h"
#include "MIDI.h"
#include "buttons.h"



#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "WS2812.hpp"


//#include "lib/PicoLED/PicoLed.hpp"

uartMidi * pianoMidi;
usbMidi * computerMidi;

uint8_t hue=0;
//WS2812 * ledStrip;


bool hueInterrupt (struct repeating_timer *t) {
    //ledStrip->show();
    hue+=1;
    if (hue==255) {
        hue=0;
    }
    
    //led->setHueFill( hue,DUAL,50);
    return true;
}



void core1() 
{
    sleep_ms(1000);
    //uint8_t ledsOn=0;
    //ledStrip = new WS2812(LED_PIN,NUM_LEDS,pio0,0);
    printf("initialise led\n");
    led = new LED();
    
    
    sleep_ms(500);
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    
    //ledStrip->fill(WS2812::RGB(0,0,0));
    //ledStrip->show();
    //struct repeating_timer timer;
    //struct repeating_timer timer2;

    //add_repeating_timer_ms(100,hueInterrupt,  NULL, &timer2));
        
    
    
    // led->setHueFill( 255,LED::DUAL,100);
    led->setSaturationFill(230,LED::DUAL);
    // led->setBrightnessFill(10,LED::DUAL,100);
    // sleep_ms(5000);
    // for (uint8_t i = 0; i <255;i++) {
    //     led->setHueFill( i,LED::DUAL,10);
    //     printf("hue %u\n",i);
    //     sleep_ms(50);
        
    // }
    
    
    //led->setBrightnessFill(0,LED::DUAL,0,500);
    led->setHueFill( 180,LED::DUAL);
    
    bool picoLed=0;
    // int64_t start=0;
    // uint8_t count=0;
    
    while (true) {
        // start = to_us_since_boot(get_absolute_time());
        // led->setHueFill(70,0);
        // led->setSaturationFill(255,0);
        while(pianoMidi->available()>0) {
            midiData message = pianoMidi->read();
            if(message.command==0x90) {
                if (message.data1<108&&message.data1>20) {
                    if (message.data2>0) {
                        
                        printf("ledOn\n");
                        
                        //led->setHue((NUM_KEYS-message.data1+FIRST_KEY),message.data2,DUAL,0,0,LED::EFFECT);
                        led->setBrightness((NUM_KEYS-message.data1+FIRST_KEY),message.data2,LED::DUAL,0,0,LED::KEY_ON);
                        led->changeBrightness((NUM_KEYS-message.data1+FIRST_KEY),-100,LED::DUAL,6000,0,LED::KEY_OFF);
                    } else {
                        
                        printf("ledOff\n");
                        
                        if (message.data1-FIRST_KEY<71) {
                            led->changeBrightness((NUM_KEYS-message.data1+FIRST_KEY),-100,LED::DUAL,500,0,LED::KEY_OFF);
                        }
                    }
                }
            }
            
            
            //midi->sendMessage(message,MIDI::target::Piano);     
            //ledStrip->setPixelColor((88-message.data1+20)*2,WS2812::RGB(0,message.data2,0));   
            // if (message.data2>0) {
            //     ledsOn ++;
            // }
            // else {
            //     ledsOn --;
            // }
            // count++;
            
            //led->onInterrupt();
            
            
        }
        // if (count==10) {
        //     printf("%llu\n",(to_us_since_boot(get_absolute_time())-start)/10);
        //     count=0;
        // }
        
        //sleep_ms(100);
        //led->show();
        picoLed=!picoLed;
        gpio_put(25, picoLed);
        
    }
    while (true) {
        
        
    }

}

void core0() 
{
   
    
    pianoMidi = new uartMidi(uart1,8,9);
    computerMidi = new usbMidi();
    
    
    while (true) {
        pianoMidi->onLoop();
        computerMidi ->onLoop();
        // led->onInterrupt();
        // sleep_ms(100);
    }
    
}



int main()
{
    stdio_init_all();
    sleep_ms(5000);

    multicore_launch_core1(core1);
    core0();


    while (true){}
    return  0;
}


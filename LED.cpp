#include "LED.h"

LED::RgbColour LED::HsvToRgb (LED::HsvColour hsv) { // https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
    RgbColour rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / (43*multiplier);
    remainder = (hsv.h - (region * (43*multiplier))) * (6*multiplier); 

    p = (hsv.v * (256*multiplier - hsv.s)) >> 15;
    q = (hsv.v * (256*multiplier - ((hsv.s * remainder) >> 15))) >> 15;
    t = (hsv.v * (256*multiplier - ((hsv.s * (255 - remainder)) >> 15))) >> 15;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

LED::LED() {

    ledStrip = new WS2812(LED_PIN,NUM_LEDS,pio0,0);
    ledStrip->fill(WS2812::RGB(0,0,0));
    
    //sleep_ms(100);
    for (int i = 0; i < NUM_LEDS; i++) {
        for (int j =0; j < 3; j++) {
            currentHSV[i][j] = 0;
            currentRGB[i][j] = 0;
        }
    }
    refreshDelay = ((uint16_t)1000)/LED_FREQUENCY;
    
    printf("command buffer size %u\n",commandQueue.size());
    //printf("add interrupt\n");
    int64_t timerDelay = 1000000/(LED_FREQUENCY*ditheringLevel);
    add_repeating_timer_us(timerDelay,repeating_timer_callback, NULL, &timer);
    //printf("added interrupt\n");
}

bool repeating_timer_callback (struct repeating_timer *t) {
    led->onInterrupt();
    return true;
}

int16_t LED::calcRate (command  currentCommand) {
    int16_t rate;

    int32_t difference = currentCommand.target-currentHSV[currentCommand.led][currentCommand.channel];
    if (currentCommand.duration>0) {
        rate = difference / currentCommand.duration;
    } else {
        if (difference >0) {
            rate = multiplier*255;
        } else if (difference <0) {
            rate = multiplier *-255;
        } else {
            rate=0;
        }
    }
    if (rate!=0) {
        
        //printf("rate %i duration %u current %i target %i difference %i\n",rate,currentCommand.duration,currentHSV[currentCommand.led][currentCommand.channel],currentCommand.target,difference);
    }
    
    return rate;
}



void LED::show() {
    //printf("start show %u\n",commandQueue.size());
    if (commandQueue.size()>0){
        for (auto & Command : commandQueue) {
            
            int16_t & current = currentHSV[Command.led][Command.channel];
            
            
            //printf("difference %u\n",Command.target-current);
            int16_t rate = calcRate(Command);
            //printf("rate found\n");
            if (Command.delay>0) {
                Command.delay--;
            } else {
                if (Command.duration>0) {
                    Command.duration--;
                }
                if (rate>0) {
                    if (Command.target-current<rate) {
                        current = Command.target;
                        Command.finished=true;
                    } else {
                        current += rate;
                    }
                } else if (rate < 0) {
                    
                    if (Command.target-current>rate) {
                        current = Command.target;
                        Command.finished=true;
                        
                    } else {
                        current += rate;
                        
                    }
                }
                
                if (current<0) {
                    current=0;
                    Command.finished=true;
                }
                
                currentHSV[Command.led][Command.channel]=current;
                
                
                HsvColour hsv;

                hsv.h = currentHSV[Command.led][H];
                hsv.s = currentHSV[Command.led][S];
                hsv.v = currentHSV[Command.led][V];

                

                RgbColour rgb = HsvToRgb(hsv);
                //RgbColour rgb;
                
                // rgb.r=Command.led;
                // rgb.g=0;
                // rgb.b=0;
                
                if (ditheringLevel>1) {
                    currentRGB[Command.led][R] = rgb.r/(multiplier/ditheringLevel);
                    currentRGB[Command.led][G] = rgb.g/(multiplier/ditheringLevel);
                    currentRGB[Command.led][B] = rgb.b/(multiplier/ditheringLevel);
                } else {
                    rgb.r = rgb.r/(multiplier);
                    rgb.g = rgb.g/(multiplier);
                    rgb.b = rgb.b/(multiplier);
                
                    ledStrip->setPixelColor(Command.led,WS2812::RGB(rgb.r,rgb.g,rgb.b));
                }
                
                
                
                
                
            }
        }

        if (commandQueue.size()>0) {
            commandQueue.remove_if([this](command Command) {
                if ((currentHSV[Command.led][Command.channel] == Command.target && Command.delay == 0) or Command.finished) {
                    return true;
                } else {
                    return false;
                }
            });
            
        }
    }
    
    
    

    
    //printf("%u\n",commandQueue.size());
}

void LED::onInterrupt() {
    // interruptStart = to_us_since_boot(get_absolute_time());
    //printf("start interrupt\n");
    if (ditheringCounter==0) {
        
        show();
    }
    
    if (ditheringLevel>1){
        dither();
    }
    ledStrip->show();
    // printf("end interrupt\n");
    // printf("%llu\n",to_us_since_boot(get_absolute_time())-interruptStart);
}

void LED::dither() {
    //printf("converted rgb\n");
    for (int i = 0; i < NUM_LEDS; i ++) {
        RgbColour rgb;
        rgb.r=currentRGB[i][R]/ditheringLevel;
        rgb.g=currentRGB[i][G]/ditheringLevel;
        rgb.b=currentRGB[i][B]/ditheringLevel;
        
        for (int j = 0; j <3; j++) {
             if (currentRGB[i][j]%ditheringLevel+ditheringCounter>=ditheringLevel) {
                 
                 switch (j) {
                    case R:
                        rgb.r+=1;
                        break;
                    case G:
                        rgb.g+=1;
                        break;
                    case B:
                        rgb.b+=1;
                        break;
                 }
             }
        }
        ledStrip->setPixelColor(i,WS2812::RGB(rgb.r,rgb.g,rgb.b));
    }

    
    ditheringCounter++;
    
    if (ditheringCounter==ditheringLevel) {
        
        ditheringCounter=0;
    }
    
}


void LED::set(uint8_t keyNum, int16_t targetVal, hsvChannelNumbers channel, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    //printf("set start\n");
    uint8_t led;
    //printf("targetVal %i\n",targetVal);
    switch (mode) {
        case DUAL:
            set(keyNum,targetVal,channel,LEFT,duration,delay,type);
            set(keyNum,targetVal,channel,RIGHT,duration,delay,type);
            return;
        case LEFT:
            led = keyNum*2;
            break;
        case RIGHT:
            led = keyNum*2+1;
            break;
        case SINGLE:
            led = keyNum;
            break;
        default:
            set(keyNum,targetVal,channel,LEFT,duration,delay,type);
            set(keyNum,targetVal,channel,RIGHT,duration,delay,type);
            return;
    }
    
    command newCommand;
    newCommand.target = targetVal*multiplier;
    newCommand.delay = delay/refreshDelay;
    //printf("targetVal %i\n",newCommand.target);
    
    newCommand.type=type;
    
    if (type==KEY_OFF) {
        if (newCommand.delay==0) {
            newCommand.delay=1;
        }
    }
    // if (newCommand.delay==0 && delay !=0) {
    //     newCommand.delay=1;
    // }
    newCommand.duration = duration/refreshDelay;
    // if (newCommand.duration==0) {
    //     newCommand.duration=1;
    // }
    newCommand.led = led;
    newCommand.channel = channel;
    //printf("led %u target %i delay %u duration %u \n",newCommand.led,newCommand.target,newCommand.delay,newCommand.duration);
    
        //printf("size %u\n",commandQueue.size());
        
    commandQueue.remove_if([=](command  Command) {
        //return true;
        if ((Command.led==newCommand.led&&Command.type!=EFFECT)&&newCommand.type!=EFFECT) {
            if (newCommand.type==KEY_ON) {
                return true;
                //printf("remove\n");
            } else if (newCommand.type==KEY_OFF&&Command.type==KEY_OFF) {
                return true;
                //printf("remove\n");
            } 
        }
        return false;
    });
    
    //printf("push back start\n");
    
    try {
        commandQueue.push_back(newCommand);
        //printf("success\n");
    } catch (const std::bad_alloc& e) {
        printf("oh dear\n");
        
    }
    
    
    
}

void LED::setHSV(uint8_t keyNum, int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    set(keyNum,hue,H,mode,duration,delay,type);
    set(keyNum,saturation,S,mode,duration,delay,type);
    set(keyNum,value,V,mode,duration,delay,type);
}

void LED::setHue(uint8_t keyNum, uint8_t hue, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    set(keyNum,hue,H,mode,duration,delay,type);
}

void LED::setSaturation(uint8_t keyNum, uint8_t saturation, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    set(keyNum,saturation,S,mode,duration,delay,type);
}
void LED::setBrightness(uint8_t keyNum, uint8_t brightness, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    set(keyNum,brightness,V,mode,duration,delay,type);
}

void LED::changeBrightness(uint8_t keyNum, int16_t change, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    set(keyNum,currentHSV[keyNum][V]/multiplier+change,V,mode,duration,delay,type);
}

void LED::setFill(uint8_t targetVal, hsvChannelNumbers channel, modes mode, uint16_t duration, uint16_t delay , commandType type) {
    uint8_t range;
    if (mode == 3) {
        range = NUM_LEDS;
    } else {
        range = NUM_KEYS;
    }
    for (int i = 0; i < range; i++) {
        set(i,targetVal,channel,mode,duration,delay,type);
    }
}

void LED::setHSVFill(int16_t hue, int16_t saturation, int16_t value, modes mode, uint16_t duration, uint16_t delay , commandType type){
    setFill(hue,H,mode,duration,delay,type);
    setFill(saturation,S,mode,duration,delay,type);
    setFill(value,V,mode,duration,delay,type);
}

void LED::setHueFill        (uint8_t hue       , modes mode, uint16_t duration, uint16_t delay , commandType type){
    setFill(hue,H,mode,duration,delay,type);
}
void LED::setSaturationFill (uint8_t saturation, modes mode, uint16_t duration, uint16_t delay , commandType type){
    setFill(saturation,S,mode,duration,delay,type);
}
void LED::setBrightnessFill (uint8_t brightness, modes mode, uint16_t duration, uint16_t delay , commandType type){
    setFill(brightness,V,mode,duration,delay,type);
}
uint8_t LED::getHue(uint8_t keyNum){
    return currentHSV[keyNum][H];
}
uint8_t LED::getSaturation(uint8_t keyNum){
    return currentHSV[keyNum][S];
}
uint8_t LED::getBrightness(uint8_t keyNum){
    return currentHSV[keyNum][V];
}

LED * led;
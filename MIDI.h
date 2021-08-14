#ifndef MIDI_H
#define MIDI_H

#include "configuration.h"


#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <vector>
#include <stdio.h>
#include "pico/mutex.h"
#include <random>



#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

struct midiData {
    uint8_t status  = 0;
    uint8_t command = 0;
    uint8_t channel = 0;
    uint8_t data1   = 0;
    uint8_t data2   = 0;
};

class circularBuffer {
    private:
        midiData buffer[BUFFER_SIZE];
        uint8_t bufferFront=0;
        uint8_t bufferBack=0;
        mutex_t bufferLock;
        uint8_t incrementIndex(uint8_t index);
        uint8_t available_internal();
    public:
        circularBuffer();
        void pushBack(midiData message); // adds to end of buffer
        midiData popFront(); // gets the data from the start of the buffer
        uint8_t available(); // how many messages are stored in this buffer
};

class midiStream {
    protected:
        void findCommandAndChannel();
        circularBuffer * inputBuffer;
        circularBuffer * outputBuffer;
        bool completeMessage=false;
        midiData currentMessage;
        virtual void writeToPort(midiData message)=0;
        virtual bool available_internal()=0;
        virtual void readFromPort()=0;
    public:
        midiStream();
        void onLoop(); // Run this every loop
        void send(midiData message);
        uint8_t available(); // If a complete message is available
        midiData read();  // returns the complete message (removing it)
};

class uartMidi : public midiStream {
    protected:
        uint8_t counter =0;
        uint8_t rawBytes[3];
        uart_inst_t * uartPort;
        void writeToPort(midiData message);
        bool available_internal();
        void readFromPort();
    public:
        uartMidi(uart_inst_t * uart_port, uint8_t txPin, uint8_t rxPin);
};

class usbMidi : public midiStream {
    protected:
        void writeToPort(midiData message);
        bool available_internal();
        void readFromPort();

    public:
        usbMidi();
};











#endif
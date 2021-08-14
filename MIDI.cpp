#include "MIDI.h"


/* ----------------------- General Midi I/O functions ----------------------- */
midiStream::midiStream() {
    inputBuffer = new circularBuffer();
    outputBuffer = new circularBuffer();
}

void midiStream::onLoop() { 
    while (available_internal()) {
        readFromPort();
        if (completeMessage) {
            findCommandAndChannel();
            inputBuffer->pushBack(currentMessage);
        }
        
    }
    while (outputBuffer->available()>0) {
        writeToPort(outputBuffer->popFront());
    }
}

void midiStream::send(midiData message) {
    if (message.status == 0) {
        message.status = message.command+message.channel;
    }
    outputBuffer->pushBack(message);
}

uint8_t midiStream::available() {
    return inputBuffer->available();
}

midiData midiStream::read() {
    return inputBuffer->popFront();
}

void midiStream::findCommandAndChannel() {
    currentMessage.channel = currentMessage.status & 0xf;
    //message.command = (message.status >> 4) & 7;
    currentMessage.command = currentMessage.status - currentMessage.channel;
}
    

/* ------------------------ UART / DIN midi functions ----------------------- */

uartMidi::uartMidi(uart_inst_t * uart_port, uint8_t txPin, uint8_t rxPin) {
    uartPort = uart_port;
    gpio_init(txPin);
    gpio_init(rxPin);
    uart_init(uartPort,31250);
    gpio_set_function(txPin, GPIO_FUNC_UART);
    gpio_set_function(rxPin, GPIO_FUNC_UART);
    inputBuffer = new circularBuffer();
    outputBuffer = new circularBuffer();
    
    #ifdef DEBUG
    printf("uartMidi started\n");
    #endif
    
}

void uartMidi::writeToPort(midiData message) {
    uart_write_blocking(uartPort,&message.status,1);
    uart_write_blocking(uartPort,&message.data1,1);
    uart_write_blocking(uartPort,&message.data2,1);
}

bool uartMidi::available_internal() {
    
    return uart_is_readable(uartPort);
}

void uartMidi::readFromPort() {
    completeMessage = false;
    uint8_t  currentByte;
    uart_read_blocking(uartPort,&currentByte,1);
    //currentByte=rand();
    #ifdef DEBUG
    //printf("byte " BYTE_TO_BINARY_PATTERN "\n",BYTE_TO_BINARY(currentByte));
    #endif
    
    switch(counter) {
        case 0:
            if (bitRead(currentByte,7)==1) {
                counter ++;
                rawBytes[0]=currentByte;
            }
            break;
        case 1:
            if (bitRead(currentByte,7)==1) {
                rawBytes[0]=currentByte;
            } else {
                rawBytes[1] = currentByte;
                counter ++;
            }
            break;
        case 2:
            if (bitRead(currentByte,7)==1) {
                counter = 1;
                completeMessage = true;
                rawBytes[2]=0;
                currentMessage.status = rawBytes[0];
                currentMessage.data1 = rawBytes[1];
                currentMessage.data2 = rawBytes[2];
                
                
                rawBytes[0]=currentByte;
                
            } else {
                counter = 0;
                rawBytes [2] = currentByte;
                
                currentMessage.status = rawBytes[0];
                currentMessage.data1 = rawBytes[1];
                currentMessage.data2 = rawBytes[2];
                completeMessage = true;
            }
            break;
    }
    #ifdef DEBUG
    //printf("counter %u\n",counter);
    #endif
    if (completeMessage) {
        
        #ifdef DEBUG    
        printf("message 0x%x 0x%x 0x%x\n", currentMessage.status,currentMessage.data1,currentMessage.data2);
        #endif
        
    }
}




/* --------------------------- USB midi functions --------------------------- */

usbMidi::usbMidi() {
    inputBuffer = new circularBuffer();
    outputBuffer = new circularBuffer();
    
    
    #ifdef DEBUG
    printf("usbMidi started\n");
    #endif
}

void usbMidi::writeToPort(midiData message) {
    
}
bool usbMidi::available_internal() {
    return false;
}
void usbMidi::readFromPort() {
    
}



/* ------------------------ Circular Buffer functions ----------------------- */

circularBuffer::circularBuffer() {
    mutex_init(&bufferLock);
}

uint8_t circularBuffer::incrementIndex(uint8_t index) {
  index++;
  if (index == BUFFER_SIZE) {
    index = 0;
  }
  return index;
}

void circularBuffer::pushBack(midiData message) {
    #ifdef DEBUG
    printf("pushBack\n");
    #endif
    //mutex_enter_blocking(&bufferLock);
    buffer[bufferBack]=message;
    if (available()==BUFFER_SIZE) {
        bufferFront=incrementIndex(bufferFront);
    }
    bufferBack =incrementIndex(bufferBack);
    //mutex_exit(&bufferLock);
}

midiData circularBuffer::popFront() {
    #ifdef DEBUG
    printf("popFront\n");
    #endif
    //mutex_enter_blocking(&bufferLock);
    midiData message = buffer[bufferFront];
    if (bufferFront==bufferBack) {
        bufferBack =incrementIndex(bufferBack);
    }
    bufferFront=incrementIndex(bufferFront); 
    //mutex_exit(&bufferLock);
    return message;
}

uint8_t circularBuffer::available_internal() {
    uint8_t size;
    if (bufferBack>bufferFront) {
        size = bufferBack-bufferFront;
    } else if (bufferBack < bufferFront) {
        size =  BUFFER_SIZE-bufferFront + bufferBack;
    } else {
        size =  0;
    }
    return size;
}

uint8_t circularBuffer::available() {
    uint8_t size;
    //mutex_enter_blocking(&bufferLock);
    size = available_internal();
    //mutex_exit(&bufferLock);
    /*
    #ifdef DEBUG
    printf("bufferBack %u \n",bufferBack);
    printf("bufferFront %u \n",bufferFront);
    printf("available %u \n",size);
    #endif
    */
    return size;
}

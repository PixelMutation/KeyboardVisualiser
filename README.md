# KeyboardVisualiser

This is a WIP RGB visualiser for manual instruments (Pianos, organs etc.) with MIDI output, which currently supports the WS2812(B) LEDs. 

It is designed to run on a Pi Pico or other RP2040 boards as it utilises both cores to allow the reading, writing and processing of midi to be simultaneous with controlling the LEDs and running various effects based upon keypresses and MIDI input.

The LED class creates a framework for easily making effects, allowing you to schedule fades between different HSV values. To ensure effects work as intended, you can set the priority of scheduled changes (e.g. so that older commands for the same LED get overwritten if they run longer than the newer command).

The program can be adapted via the configuration.h file to work with your instrument. However, the current version takes MIDI from the instrument via UART and from the computer via USB, so if your instrument only has a USB midi output, good luck? Perhaps this might make it possible to have 2 USB ports https://github.com/Noltari/pico-uart-bridge, alternatively you could try using a MIDI to USB converter to plug in your instrument.
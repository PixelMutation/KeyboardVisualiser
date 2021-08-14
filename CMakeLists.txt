# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "D:/docs/Pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(KeyboardVisualiser C CXX ASM)


set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(KeyboardVisualiser 
               KeyboardVisualiser.cpp 
               LED.cpp
               MIDI.cpp
               WS2812.cpp
               )

pico_set_program_name(KeyboardVisualiser "KeyboardVisualiser")
pico_set_program_version(KeyboardVisualiser "0.1")

pico_enable_stdio_uart(KeyboardVisualiser 0)
pico_enable_stdio_usb(KeyboardVisualiser 1)

# Add the standard library to the build
target_link_libraries(KeyboardVisualiser pico_stdlib)

# Add any user requested libraries
target_link_libraries(KeyboardVisualiser
        hardware_pio
        hardware_interp
        hardware_timer
        pico_multicore
        )



pico_add_extra_outputs(KeyboardVisualiser)

#include("lib/PicoLed/PicoLed.cmake")

pico_generate_pio_header(KeyboardVisualiser ${CMAKE_CURRENT_LIST_DIR}/WS2812.pio)


target_compile_options(KeyboardVisualiser PRIVATE -Wall -Wextra -pedantic )

#target_link_options(KeyboardVisualiser PRIVATE -fsanitize=address -static-libasan)

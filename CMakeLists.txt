cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# By default, we assume we are using a first-party Pico 2 board.
if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD "pico2")
endif()
message(STATUS "Using board ${PICO_BOARD}")

# We use the Raspberry Pi Pico C/C++ SDK. It can be installed either by using
# the official Raspberry Pi Pico Visual Studio Code extension in a compatible
# editor, or by cloning the SDK repository from
# https://github.com/raspberrypi/pico-sdk.
# In both cases, this project expects that the environment variable
# `PICO_SDK_PATH` is set to the SDK's directory (i.e., /path/to/pico-sdk).
if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR
        "PICO_SDK_PATH not defined. Is the Pico SDK installed?")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(readout_test LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR
        "Pi Pico SDK version 2.2.0 or later required. Found version "
        "${PICO_SDK_VERSION_STRING}.")
endif()

pico_sdk_init()

add_executable(main_fw
    src/main.cpp src/pins.cpp src/cli.cpp src/config_cmd.cpp
)

target_link_libraries(main_fw PRIVATE pico_stdlib hardware_spi pico_time)
target_include_directories(main_fw PRIVATE include)
target_compile_options(main_fw PRIVATE -Wall -Wextra)

# The Pico's stdio runtime library implementation is used for communicating
# with the microcontroller from a PC, primarily using the board's USB jack.
pico_enable_stdio_usb(main_fw 1)
pico_enable_stdio_uart(main_fw 0)

pico_add_extra_outputs(main_fw)

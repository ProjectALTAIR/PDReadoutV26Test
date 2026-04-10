# ALTAIR Photdiode Readout Testing Code
This repository contains source for firmware for a Raspberry Pi Pico 2
development board to be connected to the V26A revisions of the stacked pair of
ALTAIR photodiode readout boards.

## Building
These instructions are tailored to building the firmware from a Linux
environment using a manually installed Pico SDK. They may differ slightly if a
different envrionment, with a different combination of operating system and Pico
SDK installation, is used instead.

These instructions are based on Appendix C of the Raspberry Pi document
[Getting Started with Raspberry Pi Pico-series](https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008276-DS-1-getting-started-with-pico.pdf).

### Prerequisites
The following should be accessible through a system package manager.
* gcc and g++ 14 or later, including the `gcc-arm-none-eabi` toolchain
* CMake 3.21 or later
* A C++23 or later standard library implementation built using a newlib C
  standard library implementation, accessible on Ubuntu and similar
  distributions as the `libstdc++-arm-none-eabi-newlib` apt package.

### Installing the SDK
To install the SDK, navigate to a reasonable directory to hold its Git
repository, and clone its master branch.
```sh
$ git clone https://github.com/raspberrypi/pico-sdk.git --branch master
$ cd pico-sdk
$ git submodule update --init
```

This project assumes the environment variable `PICO_SDK_PATH` is set to the
`pico-sdk` directory created by the `git clone`. To set this for the current
shell session, run the following command from the pico-sdk directory.
```sh
$ export PICO_SDK_PATH="/path/to/the/pico-sdk"
```

The way to set persistent environment variables depends heavily on the Linux
environment and is not covered here.

### Configuration, building, and flashing
Clone this repository. Then, from the repository's root directory (i.e., the
directory containing this file), run the following command to configure the
build scripts.
```sh
$ cmake -S . -B build
```
This can be modified as desired to configure CMake behaviour.

Then, to build, from the same directory, run
```sh
$ cmake --build build -t main_fw
```
When the build completes successfully, this will create a file
`build/main_fw.uf2`.

Plug in a Pico 2 board with USB while holding the BOOTSEL button. It will show
up as a removable storage device. Mount this device and copy `main_fw.uf2`
created in the previous step onto the Pico 2. It will automatically disconnect
and begin running the program.

The program communicates primarily through the Pico SDK's stdio USB runtime
driver. When plugged into a computer, it will show up as a `/dev/ttyACM*`
device. A popular program to interact with the Pico 2 at this point is
`minicom`.

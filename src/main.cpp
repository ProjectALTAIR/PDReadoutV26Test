/* main.cpp
 *
 * Contains the entry point and main loop for the readout test firmware.
 *
 * Copyright (C) 2026 Project ALTAIR
 *
 * This file is part of ALTAIR Photdiode Readout V26 Testing.
 *
 * ALTAIR Photdiode Readout V26 Testing is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * ALTAIR Photdiode Readout V26 Testing is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALTAIR Photdiode Readout V26 Testing.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstdint>

#include <pico/stdlib.h>

// Local includes placeholder

int main() {
    stdio_init_all();

    // Wait for a computer to connect to the USB interface.
    while(!stdio_usb_connected()) {}

    // Wait for the computer to set up its side of the interface enough to
    // listen to our output.
    sleep_ms(1000);

    printf("Welcome to ALTAIR Photodiode Readout Testing\n");

}

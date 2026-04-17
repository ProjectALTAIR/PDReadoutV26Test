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
#include <string_view>

#include <pico/stdlib.h>

#include "altair/ads124s08.hpp"
#include "altair/cli.hpp"
#include "altair/config_cmd.hpp"
#include "altair/consteval_utils.hpp"
#include "altair/dac5311.hpp"
#include "altair/integrator.hpp"
#include "altair/pins.hpp"

int main() {
    stdio_init_all();

    // Initialize the GPIOs.
    altair::init_all_pins();

    // Create the device objects for the ADCs and DACs
    altair::SgtAdc sgt_adc {};
    altair::SdrAdc sdr_adc {};

    altair::SgtDac sgt_dac {};
    altair::SdrDac sdr_dac {};

    // Initialize command and argv buffers.
    std::array<char, altair::MAX_COMMAND_LENGTH> cmd_buf {};
    altair::ArgVType argv {};

    // We want to try and initialize both ADCs early if they're connected,
    // since a design oversight related to the 2.5V reference leaves the ADCs
    // in a less-than-great state if that reference is not active.
    //
    // The flags are named this way because this is one of the only ways to
    // know if a board is connected (and sufficiently powered).
    bool sgt_exists {sgt_adc.init_adc()};
    bool sdr_exists {sdr_adc.init_adc()};

    // Wait for a computer to connect to the USB interface.
    while(!stdio_usb_connected()) {
        // We don't have much else to do at this point, so we can try again to
        // connect to the ADCs if still needed. Otherwise we just wait for a
        // USB connection.
        if(!sgt_exists) {
            sgt_exists = sgt_adc.init_adc();
        }
        if(!sdr_exists) {
            sdr_exists = sdr_adc.init_adc();
        }
        sleep_ms(100);
    }

    // Wait for the computer to set up its side of the interface enough to
    // listen to our output.
    sleep_ms(1000);

    printf("Welcome to ALTAIR Photodiode Readout Testing\n\n");

    if(sgt_exists) {
        printf("Sergeant board detected and initialized.\n");
    } else {
        printf("Did not detect Sergeant on start-up.\n"
               "Use the `reset` or `connect` command to attempt to initialize"
               " the Sergeant board's ADC.\n");
    }

    if(sdr_exists) {
        printf("Soldier board detected and initialized.\n\n");
    } else {
        printf("Did not detect Soldier on start-up.\n"
               "Use the `reset` or `connect` command to attempt to initialize"
               " the Soldier board's ADC.\n\n");
    }

    // TODO: Add GPL notice and related commands.

    // Main program loop.
    printf("> ");
    stdio_flush();
    while(1) {
        // Poll for a command.
        std::size_t cmd_length {altair::fetch_cmd(cmd_buf)};
        if(!cmd_length) {
            // Wait a bit and then poll again.
            sleep_ms(100);
            continue;
        }

        // Tokenize the command.
        std::size_t argc {altair::split_argv(
            argv, std::string_view {cmd_buf.data(), cmd_length}
        )};
        if(!argc) {
            // This probably should not happen, but it means we do not actually
            // have a command.
            sleep_ms(100);
            continue;
        }

        // Parse the command from argv[0].
        altair::Command cmd_enum {altair::parse_command(argv[0])};

        // Run the appropriate function based on the command.
        switch(cmd_enum) {
        case altair::Command::CONNECT:
        case altair::Command::RESET: {
            if(argc == 1) {
                // Split into multiple printf calls to reuse a common
                // substring. Very minor firmware size optimization.
                printf("Error: Too few arguments.\n");
                printf("Type `help reset` for more info.\n");
                break;
            } else if(argc > 2) {
                printf("Error: Too many arguments.\n");
                printf("Type `help reset` for more info.\n");
                break;
            }
            altair::TargetBoard tgt {altair::parse_target(argv[1])};
            if(tgt == altair::TargetBoard::UNK) {
                printf("Error: Unknown target.\n");
                printf("Type `help reset` for more info.\n");
                break;
            }
            // This command doesn't need to check can_use_target because it's
            // the command that can verify which boards exist.
            tgt = altair::reset_adc(tgt, sgt_adc, sdr_adc);

            // Update what we know of boards existing.
            sgt_exists = sgt_exists || (static_cast<std::uint8_t>(tgt)
                & to_u8(altair::TargetBoard::SGT));
            sdr_exists = sdr_exists || (static_cast<std::uint8_t>(tgt)
                & to_u8(altair::TargetBoard::SDR));
            break;
        }
        case altair::Command::HELP: {
            altair::show_help(argc, argv);
            break;
        }
        default:
            // Hook into existing show_help logic to give the user the list of
            // commands and show them what they typed.
            argv[1] = argv[0];
            altair::show_help(2, argv);
            break;
        }
        // Restore the prompt.
        printf("> ");
        stdio_flush();
    }
}

/* TODO:
 * - Get main() into an init -> parse and run commands structure.
 * - Config commands (TIA gain, BV enable/voltage, integrator
 *   sequence/durations, ADC OSR/filter/single-shot, etc.)
 * - Test commands (TIA only, sweep TIA + temperature + vgnd, calibrate ADC,
 *   each integrator with the configured sequence + durations, etc.)
 * - GPLv3-related commands. (show w, show c)
 */

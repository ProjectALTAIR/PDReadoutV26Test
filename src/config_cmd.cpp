/* config_cmd.hpp
 *
 * Implements the configuration commands declared in config_cmd.hpp.
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

#include <pico/stdio.h>

#include "altair/config_cmd.hpp"

namespace altair {

TargetBoard reset_adc(
    TargetBoard tgt, SgtAdc& sgt_adc, SdrAdc& sdr_adc
) {
    // Unknown target -> Nothing to do.
    if(tgt == TargetBoard::UNK) {
        return TargetBoard::UNK;
    }

    bool reset_sgt {false}, reset_sdr {false};

    // if tgt is SGT or BOTH
    if(static_cast<std::uint8_t>(tgt) & to_u8(TargetBoard::SGT)) {
        printf("Attempting to reset the Sergeant ADC.\n"
               "Type something to cancel: ");
        stdio_flush();
        while(!reset_sgt) {
            if(stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
                // User wants to cancel. Clear the rest of stdin.
                while(stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
                printf("Cancelled.\n");
                return TargetBoard::UNK;
            }
            reset_sgt = sgt_adc.init_adc();
            sleep_ms(100);
        }
        printf("Sergeant ADC reset successfully.\n");
    }

    // if tgt is SDR or BOTH
    if(static_cast<std::uint8_t>(tgt) & to_u8(TargetBoard::SDR)) {
        printf("Attempting to reset the Soldier ADC.\n"
               "Type something to cancel: ");
        stdio_flush();
        while(!reset_sdr) {
            if(stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
                // User wants to cancel. Clear the rest of stdin.
                while(stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
                printf("Cancelled.\n");
                return reset_sgt ? TargetBoard::SGT : TargetBoard::UNK;
            }
            reset_sdr = sdr_adc.init_adc();
            sleep_ms(100);
        }
        printf("Soldier ADC reset successfully.\n");
    } else {
        // tgt was SGT
        return TargetBoard::SGT;
    }

    // tgt was SDR or BOTH
    return reset_sgt ? TargetBoard::BOTH : TargetBoard::SDR;
}

/*
// Test code, delete when info is moved to actual commands.
int example_integration() {
    namespace adc = altair::adc_enums;

    altair::SgtAdc sgt_adc {};
    altair::Acf<altair::naive_sequence> acf {};

    // Initialize GPIOs and SPI (not implemented yet)

    // Set up the ADC.
    while(!sgt_adc.init_adc()) {
        sleep_ms(100);
    }

    // Close the relay to the ACF, open IVC and low-gain TIA relays if they're
    // open.
    std::uint8_t reg {sgt_adc.read_single_register(adc::RegAddr::GPIODAT)};
    reg = (reg & 0b0001) | 0b0010;
    sgt_adc.write_single_register(adc::RegAddr::GPIODAT, reg);
    // Measure AIN4 - AIN0.
    sgt_adc.write_single_register(adc::RegAddr::INPMUX, 0x40);
    // No global chop, yes external clock, single shot, low latency,
    // 1000 OSR.
    // (Very noisy but doesn't keep the integrator waiting for quite as long.)
    sgt_adc.write_single_register(adc::RegAddr::DATARATE, 0x72);

    // Set up the integrator.
    acf.set_duration(0, -15); // reset 15us
    acf.set_duration(1, -10); // hold 10us
    acf.set_duration(2, -5000); // integrate 5ms
    // Leave last duration at 0.
    if(acf.get_duration(3) != 0) return -1;

    // Get dummy code to start things off.
    std::int32_t adc_code {0};

    // V_int - V_g = -R I_in
    // => I_in = (V_g - V_int) / R
    // V_g := 2.23k/(2.32k+2.23k) * 10V
    // R := T_int / 100pF
    // Here: T_int = 5ms
    //
    // V_o - 1.25V = -K (V_int - 1.25V)
    // => V_o = 1.25V (K + 1) - K V_int
    // => K V_int = 1.25V (K + 1) - V_o
    // => V_int = 1.25V (1 + 1/K) - V_o / K
    // => I_in = V_g / R - (1.25V (1 + 1/K) - V_o / K) / R
    //         = V_g / R - 1.25V (K + 1) / (K R) + V_o / (K R)
    // K := (55.11k || 450k) / 150k
    //
    // V_o - 2.5V = 2.5V D / 2^23
    // => V_o = 2.5V D / 2^23 + 2.5V
    // => I_in = V_g / R - 1.25V (K + 1) / (K R) + (2.5V D / 2^23 + 2.5V)/(K R)
    //         = V_g/R - 1.25V(K+1)/(K R) + 2.5V/(K R) + 2.5V D / (2^23 K R)
    //         = B + A D
    constexpr double V_g {(2230./4550.) * 10.};
    constexpr double R {5e-3/100e-12};
    constexpr double K {(55.11E3 * 450E3)/((55.11E3 + 450E3) * 150E3)};

    constexpr float B {V_g / R - 1.25*(K+1)/(K*R) + 2.5/(K*R)};
    constexpr float A {2.5/(8388608.0 * K * R)};

    if(!acf.start_integration()) return -1;

    // Wait for integration to finish.
    while(acf.is_integrating()) {
        sleep_us(10);
    }

    return 0;
}
*/

} // namespace altair

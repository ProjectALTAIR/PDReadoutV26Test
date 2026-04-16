/* pins.hpp
 *
 * Identifies the 20 RP2350 GPIO pins corresponding to the 20 wires on the
 * readout boards' system header. Also defines which peripheral(s) are used
 * and an interface to initialize all the pins appropriately.
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

#ifndef ALTAIR_PD_PINS_HPP
#define ALTAIR_PD_PINS_HPP

#include <cstdint>

#include <hardware/spi.h>

namespace altair {

// Pins
//
// Authoritative pins enum used across the codebase. Names correspond to those
// on the V26A Sergeant and Soldier schematics next to J1, the 20-pin system
// header.
enum class Pins: std::uint32_t {
    // SPI. Must correspond to the SPI pin functions that specific GPIO pins
    // have. Take care to ensure each one is on the SPI instance configured
    // below.
    SPI_SCLK = 10, // J1 Pin 14
    SPI_MOSI = 11, // J1 Pin 8
    SPI_MISO = 12, // J1 pin 11

    // SPI chip selects. Can be any pin; they are treated as GPIO rather than
    // as part of the SPI peripheral.
    SPI_CS1 = 14, // J1 pin 15
    SPI_CS2 = 15, // J1 pin 16
    SPI_CS3 = 9,  // J1 pin 12
    SPI_CS4 = 13, // J1 pin 13

    //Non-SPI ADC pins. Can be any pin.
    SYS_RESET = 1, // J1 pin 2
    DRDY1 = 5,     // J1 pin 6
    DRDY2 = 6,     // J1 pin 7
    STRT1 = 7,     // J1 pin 9
    STRT2 = 8,     // J1 pin 10

    // ACF2101 dual-channel integrator pins. Can be any pin.
    INT_SLCTA = 0, // J1 pin 1
    INT_SLCTB = 4, // J1 pin 5
    INT_RST = 2,   // J1 pin 3
    INT_HLD = 3,   // J1 pin 4

    // IVC102 single-channel integrator pins. Can be any pin.
    IVC_SW_1 = 17, // J1 pin 19
    IVC_SW_2 = 16, // J1 pin 20
    IVC_SW_1_SOL = 19, // J1 pin 17
    IVC_SW_2_SOL = 18  // J1 pin 18
};

// SPI
//
// This is the SPI instance used for both boards' ADCs and bias-voltage DACs.
// It determines which subset of pins can be used for SPI_SCLK, SPI_MOSI, and
// SPI_MISO.
inline spi_inst_t* const SPI = spi1;

// init_all_pins
//
// Initializes every pin this program uses as either simple I/O or as part of
// the SPI peripheral based on what is needed. Also drives output pins to
// reasonable initial values.
void init_all_pins();

} // namespace altair

#endif // ALTAIR_PD_PINS_HPP

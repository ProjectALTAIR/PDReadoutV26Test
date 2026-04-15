/* dac5311.hpp
 *
 * Defines a software interface for a DAC5311 buffered-output string DAC as the
 * altair::Dac class template.
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

#ifndef ALTAIR_PD_DAC5311_HPP
#define ALTAIR_PD_DAC5311_HPP

#include <cstdint>

#include "altair/pins.hpp"

namespace altair {

// DacPowerdown
//
// These enumerators correspond to the 2 power-down bits on the DAC5311, used
// to put it in a state where the output is grounded with a given impedance or
// put into high-impedance.
enum class DacPowerdown {
    NORMAL   = 0b00, // Normal Operation
    GND_1K   = 0b01, // Power-down: output to GND is 1kOhms
    GND_100K = 0b10, // Power-down: output to GND is 100kOhms.
    HIGH_Z   = 0b11  // Power-down: output is high-impedance.
};

// Dac
//
// A minimal interface to configure the DAC's output voltage and power-down
// functionality over SPI.
//
// Only 2 instances of this are needed: one for the Sergeant and one for the
// Soldier. Set the template argument where those are instantiated according to
// the populated channel selection jumpers on the boards.
template<Pins cs_pin>
class Dac {
public:
    // 3-wire SPI controls. Note that both DACs are on the same SPI bus.
    static constexpr Pins CS {cs_pin};
    static constexpr Pins MOSI {Pins::SPI_MOSI};
    static constexpr Pins SCLK {Pins::SPI_SCLK};

    // DAC resolution in bits.
    static constexpr unsigned int BITRES {8};

    // Full-scale voltage, relative to real ground.
    static constexpr float AVDD {10.0f};
    // Zero-scale voltage, relative to real ground.
    // (i.e., virtual ground to real ground.)
    static constexpr float AVSS {10.0f*(2.23f/(2.23f+2.32f))};
    // Default reverse bias setting, relative to AVSS.
    static constexpr float DEFAULT_BIAS {1.0f};

    // Full-scale voltage, relative to virtual ground.
    static constexpr float VFS {AVDD - AVSS};
    // Voltage step size between adjacent codes (assuming zero DNL).
    static constexpr float VSTEP {VFS / (1 << BITRES)};
    // Default reverse bias code.
    static constexpr std::uint8_t DEFAULT_CODE {
        static_cast<std::uint8_t>(DEFAULT_BIAS / VSTEP + 0.5f)
    };

    // Time we need to wait after taking the DAC out of power-down mode,
    // in microseconds.
    static constexpr std::uint64_t DELAY_WAKEUP {50};

    // Default constructor. Initializes the data members to their known initial
    // values, assuming the DAC has not previously been controlled. Does not
    // communicate with the DAC at all.
    Dac(): pd_ {DacPowerdown::NORMAL}, code_ {0x00} {}

    // DACs are not copyable or movable.
    Dac(Dac const&) = delete;
    Dac(Dac&&) = delete;
    Dac& operator=(Dac const&) = delete;
    Dac& operator=(Dac&&) = delete;

    // set_code
    //
    // Sends and remembers the given code to the DAC to set its output voltage.
    // Does not take the DAC out of a power-down mode if it is in one.
    //
    // code: The 8-bit code to be sent to the DAC.
    void set_code(std::uint8_t code = DEFAULT_CODE);

    // get_code
    //
    // Returns what we think the DAC's code is set to. This may not be true,
    // particularly if one or more of the DAC and MCU power-cycles or otherwise
    // resets.
    //
    // Returns the stored code from the last time set_code or
    // set_code_and_powerdown were called, or zero if neither has been called.
    std::uint8_t get_code() const { return code_; }

    // set_powerdown
    //
    // Sends and remembers a power-down state to the DAC. See the enum
    // definition for DacPowerdown for the options.
    // If we believe the DAC was previously in powerdown mode and it is set to
    // normal mode, this function stalls for DELAY_WAKEUP microseconds before
    // returning to give the DAC time to transition.
    //
    // state: The powerdown state to set the DAC to.
    void set_powerdown(DacPowerdown state);

    // get_powerdown
    //
    // Returns what we think the DAC's power-down state is. This may not be
    // true, particularly if one or more of the DAC and MCU power-cycles and
    // the DAC was put in a power-down state at some point in the past.
    //
    // Returns the stored power-down state from the last time set_powerdown or
    // set_code_and_powerdown were called, or DacPowerdown::NORMAL if neither
    // has been called.
    DacPowerdown get_powerdown() const { return pd_; }

    // set_code_and_powerdown
    //
    // Sends and remembers the given code and power-down state to the DAC. This
    // effectively gives full control of the DAC to anyone with mutable access
    // to an instance of this class.
    // Does not automatically stall when waking the DAC from power-down.
    //
    // code: The 8-bit code to be sent to the DAC.
    //
    // state: The powerdown state to set the DAC to.
    void set_code_and_powerdown(std::uint8_t code, DacPowerdown state);

    // to_code
    //
    // Converts a given voltage to the corresponding DAC code. Voltage is
    // interpreted relative to virtual ground (AVSS).
    //
    // voltage: The floating-point voltage to convert to a DAC code.
    //
    // error: If not null, then this will be set to false on a successful
    //        conversion and true otherwise.
    //
    // Returns the nearest DAC code if the voltage is representable. Returns
    // the default code (and tries to indicate an error) otherwise.
    static std::uint8_t to_code(float voltage, bool* error);

private:
    DacPowerdown pd_;
    std::uint8_t code_;
};

// Here is where the channel selection configuration can be altered as needed.

// SgtDac
//
// The DAC5311 reverse-bias-setting DAC on the Sergeant board.
using SgtDac = Dac<Pins::SPI_CS3>;

// SdrDac
//
// The DAC5311 reverse-bias-setting DAC on the Soldier board.
using SdrDac = Dac<Pins::SPI_CS4>;

} // namespace altair

#include "altair/impl/dac5311_impl.hpp"

#endif // ALTAIR_PD_DAC5311_HPP

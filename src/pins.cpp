/* pins.hpp
 *
 * Implements the pin initialization-related function(s) defined in pins.hpp.
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

#include <cstdint>

#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "altair/consteval_utils.hpp"
#include "altair/pins.hpp"

namespace altair {

void init_all_pins() {

    // Pin definitions.

    // All the pins we use.
    constexpr std::uint32_t all_pins {make_mask(
        to_u32(Pins::SPI_SCLK), to_u32(Pins::SPI_MOSI), to_u32(Pins::SPI_MISO),
        to_u32(Pins::SPI_CS1), to_u32(Pins::SPI_CS2), to_u32(Pins::SPI_CS3),
        to_u32(Pins::SPI_CS4), to_u32(Pins::SYS_RESET), to_u32(Pins::DRDY1),
        to_u32(Pins::DRDY2), to_u32(Pins::STRT1), to_u32(Pins::STRT2),
        to_u32(Pins::INT_SLCTA), to_u32(Pins::INT_SLCTB),
        to_u32(Pins::INT_RST), to_u32(Pins::INT_HLD), to_u32(Pins::IVC_SW_1),
        to_u32(Pins::IVC_SW_2), to_u32(Pins::IVC_SW_1_SOL),
        to_u32(Pins::IVC_SW_2_SOL)
    )};

    // The pins that need to be set to the SPI peripheral.
    constexpr std::uint32_t spi_pins {make_mask(
        to_u32(Pins::SPI_SCLK), to_u32(Pins::SPI_MOSI), to_u32(Pins::SPI_MISO)
    )};

    // The pins that are SIO outputs.
    constexpr std::uint32_t out_pins {make_mask(
        to_u32(Pins::SPI_CS1), to_u32(Pins::SPI_CS2), to_u32(Pins::SPI_CS3),
        to_u32(Pins::SPI_CS4), to_u32(Pins::SYS_RESET), to_u32(Pins::STRT1),
        to_u32(Pins::STRT2), to_u32(Pins::INT_SLCTA), to_u32(Pins::INT_SLCTB),
        to_u32(Pins::INT_RST), to_u32(Pins::INT_HLD), to_u32(Pins::IVC_SW_1),
        to_u32(Pins::IVC_SW_2), to_u32(Pins::IVC_SW_1_SOL),
        to_u32(Pins::IVC_SW_2_SOL)
    )};

    // The pins that are SIO inputs.
    // We don't actually need this mask but it's a good sanity check.
    constexpr std::uint32_t in_pins {make_mask(
        to_u32(Pins::DRDY1), to_u32(Pins::DRDY2)
    )};

    // The output pins that need to be driven high to start.
    constexpr std::uint32_t high_pins {make_mask(
        to_u32(Pins::SPI_CS1), to_u32(Pins::SPI_CS2), to_u32(Pins::SPI_CS3),
        to_u32(Pins::SPI_CS4), to_u32(Pins::SYS_RESET), to_u32(Pins::INT_HLD),
        to_u32(Pins::IVC_SW_1), to_u32(Pins::IVC_SW_1_SOL)
    )};

    // The output pins that need to be driven low to start.
    // We don't actually need this mask but it's a good sanity check.
    constexpr std::uint32_t low_pins {make_mask(
        to_u32(Pins::STRT1), to_u32(Pins::STRT2), to_u32(Pins::INT_SLCTA),
        to_u32(Pins::INT_SLCTB), to_u32(Pins::INT_RST), to_u32(Pins::IVC_SW_2),
        to_u32(Pins::IVC_SW_2_SOL)
    )};

    // Sanity-check static asserts.
    static_assert((out_pins ^ in_pins ^ spi_pins) == all_pins);
    static_assert((high_pins ^ low_pins) == out_pins);
    static_assert(!(spi_pins & (out_pins | in_pins)));

    // Initialize and enable all the pins.
    gpio_init_mask(all_pins);

    // Set the SPI peripheral pins.
    gpio_set_function_masked(spi_pins, GPIO_FUNC_SPI);

    // Initialize SPI with:
    // - 5MHz, should be more than slow enough for both parts.
    // - CPOL = 0, CPHA = 1 (See ADS124S08 datasheet).
    // - MSB first, which is all that the RP2350's SPI peripheral supports.
    spi_init(SPI, 5'000'000);
    spi_set_format(SPI, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

    // By default, pins initialized with gpio_init are inputs. We want
    // everything still on SIO except DRDY to be an output.
    gpio_set_dir_out_masked(out_pins);

    // We want to put the integrators in reset mode, so hold/sw1 should be high
    // and rst/sw2 should be low.
    // Other than that, we want chip selects and reset high, and we want strt
    // and the ACF selects low.
    gpio_put_masked(out_pins, high_pins);
}

} // namespace altair

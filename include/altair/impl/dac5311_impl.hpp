/* dac5311_impl.hpp
 *
 * Implements the DAC5311 software interface defined in dac5311.hpp.
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

#ifndef ALTAIR_PD_DAC5311_IMPL_HPP
#define ALTAIR_PD_DAC5311_IMPL_HPP

#include <array>
#include <cmath>

#include <pico/time.h>

#include "altair/dac5311.hpp"
#include "altair/consteval_utils.hpp"

namespace altair {

template<Pins cs_pin>
void Dac<cs_pin>::set_code(std::uint8_t code) {
    set_code_and_powerdown(code, pd_);
}

template<Pins cs_pin>
void Dac<cs_pin>::set_powerdown(DacPowerdown state) {
    DacPowerdown old_pd {pd_};
    set_code_and_powerdown(code_, state);
    if(state == DacPowerdown::NORMAL && state != old_pd) {
        sleep_us(DELAY_WAKEUP);
    }
}

template<Pins cs_pin>
void Dac<cs_pin>::set_code_and_powerdown(
    std::uint8_t code, DacPowerdown state
) {
    std::array<std::uint8_t, 2> transmit {};
    transmit[0] = static_cast<std::uint8_t>(state) << 6;
    transmit[0] |= code >> 2;
    transmit[1] = code << 6;

    gpio_put(to_u8(CS), 0);
    spi_write_blocking(SPI, transmit.data(), 2);
    gpio_put(to_u8(CS), 1);

    code_ = code;
    pd_ = state;
}

template<Pins cs_pin>
std::uint8_t Dac<cs_pin>::to_code(float voltage, bool* error) {
    constexpr float inv_vstep {1.0f / VSTEP};
    float result_f {voltage * inv_vstep};
    if(result_f < -0.5f || result_f >= 255.5f || !std::isfinite(result_f)) {
        if(error) {
            *error = true;
        }
        return DEFAULT_CODE;
    }
    if(error) {
        *error = false;
    }
    return static_cast<std::uint8_t>(result_f + 0.5f);
}

} // namespace altair

#endif // ALTAIR_PD_DAC5311_IMPL_HPP

/* integrator_impl.hpp
 *
 * Implements the Integrator class functions declared in integrator.hpp.
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

#ifndef ALTAIR_PD_INTEGRATOR_IMPL_HPP
#define ALTAIR_PD_INTEGRATOR_IMPL_HPP

#include "altair/consteval_utils.hpp"
#include "altair/integrator.hpp"

namespace altair {

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
Integrator<hld_pin, rst_pin, sequence>::Integrator():
    durations_ {}, mode_idx_ {IDX_NO_ALARM}, alarm_id_ {}
{
    durations_.fill(-10);
    durations_.back() = 0;
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
Integrator<hld_pin, rst_pin, sequence>::~Integrator() noexcept {
    if(mode_idx_ < IDX_NO_ALARM) {
        cancel_alarm(alarm_id_);
    }
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
bool Integrator<hld_pin, rst_pin, sequence>::set_duration(
    std::size_t index, std::int64_t duration
) {
    if(mode_idx_ < IDX_NO_ALARM) {
        // An integration is happening, so we do nothing.
        return false;
    } else if(index >= LENGTH) {
        // Index is out of bounds, so we do nothing.
        return false;
    }

    durations_[index] = duration;
    return true;
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
std::int64_t Integrator<hld_pin, rst_pin, sequence>::get_duration(
    std::size_t index
) const {
    if(index >= LENGTH) {
        // Index is out of bounds, which we signal with INVALID_DURATION.
        return INVALID_DURATION;
    }

    return durations_[index];
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
bool Integrator<hld_pin, rst_pin, sequence>::start_integration() {
    // Check if an integration is already happening.
    if(mode_idx_ < IDX_NO_ALARM) {
        return false;
    }
    // Start the integration.
    mode_idx_ = 0;
    // First mode 10us from now.
    alarm_id_t new_alarm {add_alarm_in_us(10, alarm_callback, this, false)};
    if(new_alarm <= 0) {
        // Didn't work.
        mode_idx_ = IDX_NO_ALARM;
        return false;
    }

    // Integration started successfully.
    alarm_id_ = new_alarm;
    return true;
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
bool Integrator<hld_pin, rst_pin, sequence>::stop_integration() {
    if(mode_idx_ >= IDX_NO_ALARM) {
        // No integration is happening.
        return false;
    }

    // Stop integration.
    bool result {cancel_alarm(alarm_id_)};
    mode_idx_ = IDX_NO_ALARM;

    // Put the integrator into reset.
    gpio_put(to_u32(HLD), to_u8(IntMode::RST_CHG) & 2);
    gpio_put(to_u32(RST), to_u8(IntMode::RST_CHG) & 1);

    return result; // Might be false if the alarm somehow didn't exist.
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
bool Integrator<hld_pin, rst_pin, sequence>::is_integrating() const {
    return mode_idx_ < IDX_NO_ALARM;
}

template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
std::int64_t Integrator<hld_pin, rst_pin, sequence>::alarm_callback(
    alarm_id_t id, void* user_data
) {
    // Check that we were called with expected user data.
    if(!user_data) {
        // No user data.
        return 0;
    }
    // We assume user_data is an Integrator and check its alarm ID. Not 100%
    // safe, but we can only do so much to validate a void*.
    Integrator* i {reinterpret_cast<Integrator*>(user_data)};
    if(i->alarm_id_ != id) {
        // Not the right integrator or not an integrator.
        return 0;
    }

    // We probably have the right integrator now.

    // NOTE the read-modify-write here. If we introduce more concurrency, like
    // using the second Arm core, this can and will cause issues.
    std::size_t mode_idx {i->mode_idx_};

    // If mode index is IDX_NO_ALARM or greater, the integration is over, and
    // the user presumably forgot to keep the last duration as zero.
    if(mode_idx >= IDX_NO_ALARM) {
        return 0;
    }

    // Update the Integrator and set the next alarm.
    i->mode_idx_ = mode_idx + 1;

    std::uint8_t mode_bits {static_cast<std::uint8_t>(sequence[mode_idx])};
    gpio_put(to_u32(HLD), mode_bits & 2);
    gpio_put(to_u32(RST), mode_bits & 1);

    return i->durations_[mode_idx];
}

} // namespace altair

#endif // ALTAIR_PD_INTEGRATOR_IMPL_HPP

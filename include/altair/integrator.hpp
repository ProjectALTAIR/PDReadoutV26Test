/* integrator.hpp
 *
 * Defines an interface for operating the switched integrators on the
 * readout boards with runtime-configurable integration, hold, and reset times
 * and compile-time-configurable sequences of those operations.
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

#ifndef ALTAIR_PD_INTEGRATOR_HPP
#define ALTAIR_PD_INTEGRATOR_HPP

#include <array>
#include <concepts>
#include <cstdint>

#include <pico/time.h>

#include "altair/pins.hpp"

namespace altair {

// IntMode
//
// This enum represents the states of the two switches that control an
// integrator acording to the following table.
//
// Switches are HLD and RST (SW1 and SW2 or HOLD and RESET, respectively).
// 1 = OFF/OPEN, 0 = ON/CLOSED
//
//        Mode      |HLD|RST| Note
// -----------------+---+---+--------------------------------------------------
//     Integrate    | 0 | 1 | V(t) = -1/C_int int_{t_0}^{t} i_in(T) dT
//        Hold      | 1 | 1 | V(t) = V(t_0) - t V_droop (~1mV/s)
//   Reset (Charge) | 1 | 0 |   Input current can't go to integrator; it
//                  |   |   | accumulates on sensor capacitance or (in our
//                  |   |   | case) goes to another AFE path like the TIA.
// Reset (Discharge)| 0 | 0 |   Input current is mostly passed through the
//                  |   |   | device.
enum class IntMode: std::uint8_t {
    INTEGRATE = 0b01,
    HOLD      = 0b11,
    RST_CHG   = 0b10,
    RST_DIS   = 0b00
};

// integrator_sequence
//
// This template defines a sequence of one or more integrator modes making up
// an integration cycle. The last mode in the sequence must be the Hold mode
// where an ADC is configured to measure the integrated charge.
template<IntMode... T> requires((T == IntMode::HOLD), ...)
constexpr std::array<IntMode, sizeof...(T)> integrator_sequence {T...};

// A couple of example sequences to show how the above template can be used.

// naive_sequence: Simple integrate -> hold + sample -> reset -> hold loop.
// This is what is recommended most directly by the ACF2101 and IVC102
// datasheets, though they assume the output can be sampled in as little as 10
// microseconds.
constexpr auto naive_sequence {integrator_sequence<
    IntMode::RST_CHG, IntMode::HOLD, IntMode::INTEGRATE, IntMode::HOLD
>};

// double_sequence: This alternates integration-time-length integrate cycles
// with shorter integrate cycles that aim to just dissipate charge built up on
// the sensor while we were sampling the last measurement.
constexpr auto double_sequence {integrator_sequence<
    IntMode::RST_CHG, IntMode::HOLD, IntMode::INTEGRATE, IntMode::HOLD,
    IntMode::RST_CHG, IntMode::HOLD, IntMode::INTEGRATE, IntMode::HOLD
>};

// Integrator
//
// This class template allows for configuring and operating the integrator with
// the given hold and reset pins. This is compatible with both the ACF2101 and
// the IVC102. With the ACF2101, it assumes the select switches are always
// closed and that the user will command both boards' ADCs to read their
// respective ACF2101 channels simultaneously. This is because the two sides'
// hold and reset switches are hard-wired together on the Sergeant.
//
// sequence: The integrator_sequence instantiation determining how an
//           integration cycle looks for this integrator.
//
// The interface is designed to be flexible in the timing, but not the
// ordering, of the sequence of modes. To evaluate different sequence orders,
// different types will need to be made from this class accordingly.
template<Pins hld_pin, Pins rst_pin, auto sequence>
requires(std::same_as<typename decltype(sequence)::value_type, IntMode>)
class Integrator {
public:
    // Hold and reset pins.
    static constexpr Pins HLD {hld_pin};
    static constexpr Pins RST {rst_pin};

    // Type alias for the constant-expression container defining the sequence.
    using SequenceType = decltype(sequence);

    // The number of modes in the integration sequence.
    static constexpr std::size_t LENGTH {sequence.size()};

    // A sentinel value used by get_duration to signal invalid input.
    static constexpr std::int64_t INVALID_DURATION {0xFFFF'FFFF};

    // Default constructor.
    //
    // Initializes the mode sequence index, and sets all the durations to a
    // default of 10us (except the last one, which should stay at 0us).
    //
    // This does not touch the GPIO pins because they might not yet be
    // initialized.
    Integrator();

    // Integrator is not copyable or movable.
    Integrator(Integrator const&) = delete;
    Integrator(Integrator&&) = delete;
    Integrator& operator=(Integrator const&) = delete;
    Integrator& operator=(Integrator&&) = delete;

    // Destructor.
    //
    // If a sequence of alarms is running, cancels it before exiting. Does not
    // touch GPIOs, so the integrator could potentially be left in any state.
    ~Integrator() noexcept;

    // set_duration
    //
    // Sets the duration of the indexed mode, if integration is not currently
    // happening.
    //
    // Note that this uses the same sign convention as the Pico SDK: positive
    // values are microseconds from when the last alarm callback ended, and
    // negative values are from when the last alarm was scheduled to fire.
    //
    // index: The index (starting at 0) of the mode to set a duration for.
    //
    // duration: The time (in microseconds, see above note on sign convention)
    //           that the integrator should stay in this mode before switching
    //           to the next one.
    //
    // Returns true if the index was in bounds and no integration was
    // happening, in which case the duration was set successfully. Returns
    // false otherwise.
    bool set_duration(std::size_t index, std::int64_t duration);

    // get_duration
    //
    // Retrieves the duration set for the given index.
    //
    // Returns the duration for the index, or INVALID_DURATION if the
    // index is out of bounds.
    std::int64_t get_duration(std::size_t index) const;

    // start_integration
    //
    // Begins the chain of alarms that runs an integration cycle. Because the
    // ADS124S08 ADC we use is not particularly fast at sampling (at least if
    // we want low noise), this is inherently single-shot and must be called
    // once for every measurement.
    //
    // After calling this, keep polling is_integrating. The ADC can run
    // conversions when is_integrating returns false.
    //
    // Returns true if no integration was previously happening and one
    // successfully started. Returns false if an integration was happening or
    // one could not be initiated.
    bool start_integration();

    // stop_integration
    //
    // Stops an integration early if one is happening and puts the integrator
    // into reset (charge) mode.
    // Does nothing if no integration was happening.
    //
    // Returns true if an integration was happening and was cancelled and false
    // otherwise.
    bool stop_integration();

    // is_integrating
    //
    // Indicates whether an integration cycle is in progress.
    //
    // Returns true if an integration cycle is in progress and false otherwise.
    bool is_integrating() const;

private:
    // Sentinel value for mode_idx_.
    static constexpr std::size_t IDX_NO_ALARM {LENGTH};

    // alarm_callback
    //
    // This is passed to the Pico SDK with the user data being `this`
    // to have the ISR able to update an instance of the struct as it drives
    // the GPIOs according to the sequence.
    static std::int64_t alarm_callback(alarm_id_t id, void* user_data);

    std::array<std::int64_t, LENGTH> durations_; // in microseconds.

    // This is used both to know how far along in the integration cycle we are,
    // and to indicate if alarms are currently active.
    volatile std::size_t mode_idx_;

    // We hold onto the alarm ID to be able to cancel the alarm when we stop
    // integration.
    alarm_id_t alarm_id_;
};

// Convenience templates for the 3 different integrators.
template<auto sequence>
using Acf = Integrator<Pins::INT_HLD, Pins::INT_RST, sequence>;

template<auto sequence>
using SgtIvc = Integrator<Pins::IVC_SW_1, Pins::IVC_SW_2, sequence>;

template<auto sequence>
using SdrIvc = Integrator<Pins::IVC_SW_1_SOL, Pins::IVC_SW_2_SOL, sequence>;

} // namespace altair

#include "altair/impl/integrator_impl.hpp"

#endif // ALTAIR_PD_INTEGRATOR_HPP

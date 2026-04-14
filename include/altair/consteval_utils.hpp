/* consteval_utils.hpp
 *
 * Contains a variety of compile-time utilities for convenience throughout the
 * codebase.
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

#ifndef ALTAIR_PD_CONSTEVAL_UTILS_HPP
#define ALTAIR_PD_CONSTEVAL_UTILS_HPP

#include <concepts>
#include <cstdint>

namespace altair {

// to_u32
//
// Casts `value` to an equivalent value as a 32-bit unsigned integer.
// Primarily used to turn scoped enumerators into their underlying values.
//
// Returns a 32-bit unsigned integer representation of `value`.
template<typename T> requires(requires(T v) {static_cast<std::uint32_t>(v);})
consteval std::uint32_t to_u32(T value) {
    return static_cast<std::uint32_t>(value);
}

// to_u8
//
// Casts `value` to an equivalent value as an 8-bit unsigned integer.
// Primarily used to turn scoped enumerators into their underlying values.
//
// Returns an 8-bit unsigned integer representation of `value`.
template<typename T> requires(requires(T v) {static_cast<std::uint8_t>(v);})
consteval std::uint8_t to_u8(T value) {
    return static_cast<std::uint8_t>(value);
}

// make_mask
//
// Creates a 32-bit mask from 1 or more 32-bit unsigned integer arguments.
// Will not compile if any argument is greater than 31.
//
// Returns the 32-bit value with each bit specified by an argument set and the
// other bits not set.
template<typename... Ts> requires(std::same_as<Ts, std::uint32_t> && ...)
consteval std::uint32_t make_mask(Ts... pins) {
    return ((1u << pins) | ...);
}

} // namespace altair

#endif // ALTAIR_PD_CONSTEVAL_UTILS_HPP

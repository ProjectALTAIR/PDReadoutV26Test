/* cli.hpp
 *
 * Declares functions and constants for processing a
 * command-line-interface-like input.
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

#ifndef ALTAIR_PD_CLI_HPP
#define ALTAIR_PD_CLI_HPP

#include <array>
#include <string_view>

namespace altair {

// The expected capacity of the command input buffer, in bytes.
constexpr std::size_t MAX_COMMAND_LENGTH {128};

// The maximum number of words/tokens a command can contain, and the expected
// capacity of the argument vector (argv) buffer.
constexpr std::size_t MAX_ARGC {16};

// fetch_cmd
//
// Checks for a char in stdin. If there is one, keeps grabbing characters until
// a carriage return, newline, or null terminator, or until no more characters
// are available. Populates `cmd` until either one of the aforementioned
// conditions occurred, or MAX_COMMAND_LENGTH - 1 bytes have been written
// to `cmd` (discarding further input in the latter case). Finishes by
// null-terminating `cmd` and returning the position of the null terminator.
//
// cmd: The buffer where the command is placed.
//
// Returns 0 if no characters were available. Returns the number of non-null
// characters written to `cmd` if characters were available.
std::size_t fetch_cmd(std::array<char, MAX_COMMAND_LENGTH>& cmd);

// split_argv
//
// Splits a string view of a command buffer `cmd` into tokens, which are
// contiguous substrings of characters that do not satisfy std::isspace. Up to
// the first MAX_ARGC such tokens are stored as string views in `argv`.
//
// argv: The array of token string views to be filled.
//
// cmd: The buffer containing a command to tokenize.
//
// Returns the number of tokens found (always in the range [1, MAX_ARGC]), or 0
// if `cmd` was completely empty.
std::size_t split_argv(
    std::array<std::string_view, MAX_ARGC>& argv, std::string_view cmd
);

} // namespace altair

#endif // ALTAIR_PD_CLI_HPP

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
#include <cstdint>
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

// ArgVType
//
// Convenience type alias for an array of MAX_ARGC string views, used for
// argument lists here.
using ArgVType = std::array<std::string_view, MAX_ARGC>;

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
std::size_t split_argv(ArgVType& argv, std::string_view cmd);

// TargetBoard
//
// Command functions taking this as an argument are set up to work with either
// the Sergeant or Soldier separately, or both simultaneously.
enum class TargetBoard: std::uint8_t {
    UNK = 0b00, // Unknown
    SGT = 0b01, // Sergeant
    SDR = 0b10, // Soldier
    BOTH = 0b11 // Both boards
};

// parse_target
//
// Tries to parse the input string as either "sgt", "sdr", or "both".
//
// arg: The string view to parse as a target argument.
//
// Returns the corresponding TargetBoard enumerator for the argument. All
// options other than "sgt", "sdr", or "both" parse as TargetBoard::UNK.
TargetBoard parse_target(std::string_view arg);

// can_use_target
//
// Tests if the selected target is suitable for a command that depends on the
// targeted boards existing.
//
// tgt: The target boards for which a command is to be run.
//
// sgt_exists: If true, the Sergeant board is assumed to exist.
//
// sdr_exists: If true, the Soldier board is assumed to exist.
//
// Returns true if, based on the passed flags, a command depending on the
// targeted boards existing can run.
bool can_use_target(TargetBoard tgt, bool sgt_exists, bool sdr_exists);

// Command
//
// An enum of all the implemented commands.
enum class Command: std::uint8_t {
    UNKNOWN = 0, // Unknown
    RESET,       // Reset one or both ADCs.
    CONNECT,     // Alias for RESET.
    HELP,        // Display all commands or get help on a specific command.
};

// parse_command
//
// Tries to parse the input string as one of the recognized commands.
//
// arg: The string view to parse as a target argument.
//
// Returns the corresponding Command enumerator for the argument. All options
// other than a valid command (matching its enumerator identifier, but all in
// lowercase) parse as Command::UNKNOWN.
Command parse_command(std::string_view arg);

// show_help
//
// If argc is 2 and argv[1] parses as a known command, displays a synopsis and
// description for that command. Otherwise, displays a list of commands with
// short descriptions.
//
// argc: The number of arguments in the list.
// argv: The list of arguments (including the original command).
void show_help(std::size_t argc, ArgVType const& argv);

} // namespace altair

#endif // ALTAIR_PD_CLI_HPP

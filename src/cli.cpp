/* cli.cpp
 *
 * Implements the I/O and text processing functions specified in cli.hpp.
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

#include <array>
#include <cctype>
#include <cstdio>
#include <string_view>

#include <pico/stdio.h>

#include "altair/cli.hpp"

namespace altair {

namespace {

inline bool is_part_of_cmd(int ch) {
    return ch > 0 && ch != '\r' && ch != '\n';
}

} // anonymous namespace


std::size_t fetch_cmd(std::array<char, MAX_COMMAND_LENGTH>& cmd) {
    int ch {stdio_getchar_timeout_us(0)};
    std::size_t len {0};

    // Both loops are skipped if there is nothing in the buffer or if
    // there was a newline.

    // This loop fills the command buffer if there is something to put in it.
    for(; len < MAX_COMMAND_LENGTH - 1 && is_part_of_cmd(ch); ++len) {
        cmd[len] = static_cast<char>(ch);
        ch = stdio_getchar();
    }

    // This loop discards the rest of a potentially too long command.
    for(; is_part_of_cmd(ch); ch = stdio_getchar()) {}

    // Null-terminate the buffer. (Not strictly necessary for how we use it.)
    cmd[len] = '\0';

    // Return the position of the null terminator.
    return len;
}

std::size_t split_argv(
    ArgVType& argv, std::string_view cmd
) {
    if(cmd.empty()) {
        return 0; // No tokens.
    }

    auto token_start {cmd.begin()};
    std::size_t argc {0};

    for(auto iter {cmd.begin()};
        iter != cmd.end() && argc < MAX_ARGC; ++iter
    ) {
        if(std::isspace(static_cast<unsigned char>(*iter))) {
            // This character is not part of a token. If token_start is not
            // equal to i, it's because we just finished going through a token
            // that we need to add to argv.
            if(token_start != iter) {
                argv[argc++] = std::string_view {token_start, iter};
            }
            token_start = iter + 1;
        }
    }

    // Tokenize the end of the string if we need to.
    if(token_start != cmd.end() && argc < MAX_ARGC) {
        argv[argc++] = std::string_view {token_start, cmd.end()};
    }

    return argc;
}

TargetBoard parse_target(std::string_view arg) {
    using namespace std::string_view_literals;

    if(arg == "sgt"sv) { return TargetBoard::SGT; }
    if(arg == "sdr"sv) { return TargetBoard::SDR; }
    if(arg == "both"sv) { return TargetBoard::BOTH; }

    return TargetBoard::UNK;
}

Command parse_command(std::string_view arg) {
    using namespace std::string_view_literals;

    if(arg == "reset"sv) { return Command::RESET; }
    if(arg == "connect"sv) { return Command::CONNECT; }
    if(arg == "help"sv) { return Command::HELP; }

    return Command::UNKNOWN;
}

void show_help(std::size_t argc, ArgVType const& argv) {
    if(argc == 2) {
        // Try to display info for a specific command.
        Command cmd {parse_command(argv[1])};
        switch(cmd) {
        case Command::RESET:
        case Command::CONNECT:
            printf(
                "SYNOPSIS: reset|connect {sgt | sdr | both}\n"
                "- Repeatedly attempts to reset the specified ADCs until it\n"
                "  is successful. If it stalls, type something to cancel.\n"
                "- The two commands are exactly identical..\n\n"
            );
            return;
        case Command::HELP:
            printf(
                "SYNOPSIS: help [<cmd>]\n"
                "- Without <cmd>, displays a list of commands.\n"
                "- If <cmd> is a valid command, shows a synopsis and\n"
                "  description for that command.\n\n"
            );
            return;
        default: break;
        }
        // Unknown command. Copy it to a buffer to be able to echo it back to
        // the user.
        std::array<char, MAX_COMMAND_LENGTH> cmd_buffer {};
        std::size_t l {argv[1].copy(
            cmd_buffer.data(), MAX_COMMAND_LENGTH - 1
        )};
        cmd_buffer[l] = '\0';
        printf("Unknown command: '%s'\n\n", cmd_buffer.data());
        // Fall through to displaying the list of commands.
    }

    printf(
        // List commmands.
        "COMMANDS:\n"
        "  connect    Alias for `reset`.\n"
        "  help       Show this message or specific command info.\n"
        "  reset      Attempt to connect to and reset an ADC.\n"
        // Extra info.
        "Type `help cmd` to show more detailed info on the command `cmd`.\n\n"
    );
}

bool can_use_target(TargetBoard tgt, bool sgt_exists, bool sdr_exists) {
    std::uint8_t good_mask {static_cast<std::uint8_t>(sgt_exists ? 0b01 : 0)};
    good_mask |= (sdr_exists ? 0b10 : 0);
    return (static_cast<std::uint8_t>(tgt) & good_mask)
        == static_cast<std::uint8_t>(tgt);
}


} // namespace altair

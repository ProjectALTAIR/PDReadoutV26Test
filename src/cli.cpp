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
        ch = stdio_getchar_timeout_us(0);
    }

    // This loop discards the rest of a potentially too long command.
    for(; is_part_of_cmd(ch); ch = stdio_getchar_timeout_us(0)) {}

    // Null-terminate the buffer. (Not strictly necessary for how we use it.)
    cmd[len] = '\0';

    // Return the position of the null terminator.
    return len;
}

std::size_t split_argv(
    std::array<std::string_view, MAX_ARGC>& argv, std::string_view cmd
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

} // namespace altair

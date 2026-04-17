/* config_cmd.hpp
 *
 * Declares functions for the configuration-related commands the user can
 * input.
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

#ifndef ALTAIR_PD_CONFIG_CMD_HPP
#define ALTAIR_PD_CONFIG_CMD_HPP

#include "altair/ads124s08.hpp"
#include "altair/cli.hpp"

namespace altair {

// reset_adc
//
// Attempts to reset/initialize to the specified ADC. The user can cancel
// the attempt by typing anything into stdin if the reset does not happen
// immediately. If that happens, stdin is read repeatedly to clear the buffer
// so that further input does not interfere with the next command.
//
// tgt: The board(s) to connect to. Supports sgt, sdr, both.
//
// sgt_adc: The ADC for the Sergeant.
//
// sdr_adc: The ADC for the Soldier.
//
// Returns tgt if the connection was successful. Returns TargetBoard::UNK
// if the attempt was cancelled. If tgt is TargetBoard::BOTH and the attempt
// is cancelled after the Sergeant connects, returns TargetBoard::SGT.
TargetBoard reset_adc(
    TargetBoard tgt, SgtAdc& sgt_adc, SdrAdc& sdr_adc
);

} // namespace altair

#endif // ALTAIR_PD_CONFIG_CMD_HPP

/* ads124s08.hpp
 *
 * Defines a software interface for an ADS124S08 ADC as the altair::Adc class
 * template.
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

#ifndef ALTAIR_PD_ADS124S08_HPP
#define ALTAIR_PD_ADS124S08_HPP

#include <array>
#include <cstdint>

#include "altair/pins.hpp"

namespace altair {

// This namespace contains all the scoped enum types that the ADCs use. They
// can be accessed from this namespace or from the namespace of your
// instantiated ADC type, since the below class template aliases these enums.
namespace adc_enums {

////////////////////////////
// Command-related enums. //
////////////////////////////
// BEGIN

// Opcode
//
// Every outgoing serial communication with the ADC begins with one of
// these opcode bytes.
enum class Opcode: std::uint8_t {
    // SPI Control Commands
    NOP        = 0x00,
    WAKEUP     = 0x02,
    POWERDOWN  = 0x04,
    RESET      = 0x06,
    START      = 0x08,
    STOP       = 0x0A,

    // SPI Calibration Commands
    SYOCAL     = 0x16,
    SYGCAL     = 0x17,
    SFOCAL     = 0x18,

    // SPI Data Read Command
    RDATA      = 0x12,

    // SPI Register Read and Write Commands
    RREG       = 0x20,
    WREG       = 0x40,
    RWREG_MASK = 0x1F
};

// ReadMode
//
// The ADS124S08's data can either be read directly by keeping SPI MOSI low
// while clocking in the data from SPI MISO, or by sending a RDATA command
// and then clocking in the data.
enum class ReadMode {
    DIRECT,
    COMMAND
};

// RegAddr
//
// Register addresses. The ADC has 18 configuration-related registers,
// mostly configured with WREG commands.
enum class RegAddr: std::uint8_t {
    ID       = 0x00,
    STATUS   = 0x01,
    INPMUX   = 0x02,
    PGA      = 0x03,
    DATARATE = 0x04,
    REF      = 0x05,
    IDACMAG  = 0x06,
    IDACMUX  = 0x07,
    VBIAS    = 0x08,
    SYS      = 0x09,
    OFCAL0   = 0x0A,
    OFCAL1   = 0x0B,
    OFCAL2   = 0x0C,
    FSCAL0   = 0x0D,
    FSCAL1   = 0x0E,
    FSCAL2   = 0x0F,
    GPIODAT  = 0x10,
    GPIOCON  = 0x11
};

// END


///////////////////////////////////////////
// Register field description and enums. //
///////////////////////////////////////////
// BEGIN

// What follows is a description of fields in the various registers. The syntax
// FOO[X:Y] refers to bits Y to X inclusive in the register FOO.

// ID[2:0]: DEV_ID
enum class DevID {
    ADS124S08 = 0x00,
    ADS124S06 = 0x01,
    ADS114S08 = 0x04,
    ADS114S06 = 0x05
};

// STATUS bits mask
enum class StatusMask {
    FL_POR     = 0x80,
    nRDY       = 0x40,
    FL_P_RAILP = 0x20,
    FL_P_RAILN = 0x10,
    FL_N_RAILP = 0x08,
    FL_N_RAILN = 0x04,
    FL_REF_L1  = 0x02,
    FL_REF_L0  = 0x01
};

// INPMUX[7:4]: MUXP, positive input channels.
enum class PChannels {
    AIN0   = 0x00,
    AIN1   = 0x10,
    AIN2   = 0x20,
    AIN3   = 0x30,
    AIN4   = 0x40,
    AIN5   = 0x50,
    AIN6   = 0x60,
    AIN7   = 0x70,
    AIN8   = 0x80,
    AIN9   = 0x90,
    AIN10  = 0xA0,
    AIN11  = 0xB0,
    AINCOM = 0xC0
};

// INPMASK[3:0]: MUXN, negative input channels.
enum class NChannels {
    AIN0   = 0x00,
    AIN1   = 0x01,
    AIN2   = 0x02,
    AIN3   = 0x03,
    AIN4   = 0x04,
    AIN5   = 0x05,
    AIN6   = 0x06,
    AIN7   = 0x07,
    AIN8   = 0x08,
    AIN9   = 0x09,
    AIN10  = 0x0A,
    AIN11  = 0x0B,
    AINCOM = 0x0C
};

// PGA[7:5]: DELAY, conversion delay, in tmod clock periods.
enum class DelayMode {
    D_14   = 0x00,
    D_25   = 0x20,
    D_64   = 0x40,
    D_256  = 0x60,
    D_1024 = 0x80,
    D_2048 = 0xA0,
    D_4096 = 0xC0,
    D_1    = 0xE0
};

// PGA[4:3]: PGA_EN, PGA control.
enum class PgaCtrl {
    BYPASS  = 0x00,
    ENABLED = 0x08
};

// PGA[2:0]: GAIN, gain setting
enum class GainMode {
    G_1   = 0x00,
    G_2   = 0x01,
    G_4   = 0x02,
    G_8   = 0x03,
    G_16  = 0x04,
    G_32  = 0x05,
    G_64  = 0x06,
    G_128 = 0x07,
    MASK  = 0x07
};

// DATARATE[7:4] bits mask
enum class DatarateMask {
    GLOBALCHOP    = 0x80,
    CLKSEL_EXT    = 0x40,
    CONVMODE_SS   = 0x20,
    CONVMODE_CONT = 0x00,
    FILTERTYPE_LL = 0x10
};

// DATARATE[3:0]: Define the data rate. (in samples/s)
enum class DatarateMode {
    DR_2_5  = 0x00,
    DR_5    = 0x01,
    DR_10   = 0x02,
    DR_16   = 0x03,
    DR_20   = 0x04,
    DR_50   = 0x05,
    DR_60   = 0x06,
    DR_100  = 0x07,
    DR_200  = 0x08,
    DR_400  = 0x09,
    DR_800  = 0x0A,
    DR_1000 = 0x0B,
    DR_2000 = 0x0C,
    DR_4000 = 0x0D
};

// REF[7:6]: Fl_REF_EN,
// REF[5:4]: nREFP_BUF,
// REF[3:2]: REFSEL,
// REF[1:0]: REFCON
enum class RefSetting {
    // Configures the reference monitor, which correspond to bits in the STATUS
    // register to detect issues with *external* references.
    FLAG_REF_DISABLE = 0x00,
    FLAG_REF_EN_L0   = 0x40,
    FLAG_REF_EN_BOTH = 0x80,
    FLAG_REF_EN_10M  = 0xC0,

    // Can configure to bypass voltage buffers on external reference inputs.
    REFP_BYP_DISABLE = 0x20,
    REFP_BYP_ENABLE  = 0x00,
    REFN_BYP_DISABLE = 0x10,
    REFN_BYP_ENABLE  = 0x00,

    // Select which reference input source is used for conversions.
    REFSEL_P0  = 0x00,
    REFSEL_P1  = 0x04,
    REFSEL_INT = 0x08,

    // Configure if the internal reference is turned on.
    REFINT_ON_PDWN   = 0x01, // Internal reference onexcept in power-down mode.
    REFINT_ON_ALWAYS = 0x02  // Internal reference always on.
};

// "IDAC" refers to the ADC's ability to drive a constant current through up to
// 2 channels.

// IDACMAG[7]: FL_RAIL_EN,
// IDACMAG[6]: PSW,
// IDACMAG[5:4]: 0b00,
// IDACMAG[3:0]: IMAG
enum class IdacmagSetting {
    // Configure a flag bit in the STATUS register corresponding to the PGA
    // output voltage being too close to AVDD or AVSS.
    FLAG_RAIL_ENABLE  = 0x80,
    FLAG_RAIL_DISABLE = 0x00,

    // Configures a low-side power switch between REFN0 and AVSS-SW to help
    // with measuring a resistive bridge circuit.
    PSW_OPEN   = 0x00,
    PSW_CLOSED = 0x40,

    // IDAC current in microamps.
    MAG_OFF  = 0x00,
    MAG_10   = 0x01,
    MAG_50   = 0x02,
    MAG_100  = 0x03,
    MAG_250  = 0x04,
    MAG_500  = 0x05,
    MAG_750  = 0x06,
    MAG_1000 = 0x07,
    MAG_1500 = 0x08,
    MAG_2000 = 0x09
};

// IDACMUX[7:4]: IDAC2 output pin
enum class Idac2Output {
    A0     = 0x00,
    A1     = 0x10,
    A2     = 0x20,
    A3     = 0x30,
    A4     = 0x40,
    A5     = 0x50,
    A6     = 0x60,
    A7     = 0x70,
    A8     = 0x80,
    A9     = 0x90,
    A10    = 0xA0,
    A11    = 0xB0,
    AINCOM = 0xC0,
    OFF    = 0xF0
};

// IDACMUX[3:0]: IDAC1 output pin
enum class Idac1Output {
    A0     = 0x00,
    A1     = 0x01,
    A2     = 0x02,
    A3     = 0x03,
    A4     = 0x04,
    A5     = 0x05,
    A6     = 0x06,
    A7     = 0x07,
    A8     = 0x08,
    A9     = 0x09,
    A10    = 0x0A,
    A11    = 0x0B,
    AINCOM = 0x0C,
    OFF    = 0x0F
};

// Some of the ADS124S08 pins can drive a bias voltage with limited
// flexibility. This is controlled in the VBIAS register.
enum class VbiasSetting {
    // Level: Either (VDD-VSS)/2 or (VDD-VSS)/12
    LVL_DIV2  = 0x00,
    LVL_DIV12 = 0x80,
    // Output pin
    AINC = 0x40,
    AIN5 = 0x20,
    AIN4 = 0x10,
    AIN3 = 0x08,
    AIN2 = 0x04,
    AIN1 = 0x02,
    AIN0 = 0x01
};

// SYS[7:5]: SYS_MON
// SYS[4:3]: CAL_SAMP
// SYS[2]: TIMEOUT
// SYS[1]: CRC
// SYS[0]: SENDSTAT
enum class SysSetting {
    SYS_MON_OFF    = 0x00,
    SYS_MON_SHORT  = 0x20,
    SYS_MON_TEMP   = 0x40,
    SYS_MON_ADIV4  = 0x60,
    SYS_MON_DDIV4  = 0x80,
    SYS_MON_BCS_2  = 0xA0,
    SYS_MON_BCS_1  = 0xC0,
    SYS_MON_BCS_10 = 0xE0,

    CALSAMPLE_1  = 0x00,
    CALSAMPLE_2  = 0x08,
    CALSAMPLE_4  = 0x10,
    CALSAMPLE_8  = 0x18,
    CALSAMPLE_16 = 0x18,

    TIMEOUT_DISABLE = 0x00,
    TIMEOUT_ENABLE  = 0x04,

    CRC_DISABLE = 0x00,
    CRC_ENABLE  = 0x02,
    CRC_MASK    = 0x02,

    SENDSTATUS_DISABLE = 0x00,
    SENDSTATUS_ENABLE  = 0x01,
    SENDSTATUS_MASK    = 0x01
};

// GPIODAT[7:4]: Set I/O direction (1=input).
enum class GpioDir {
    GPIO0 = 0x10,
    GPIO1 = 0x20,
    GPIO2 = 0x40,
    GPIO3 = 0x80
};

// GPIODAT[3:0]: GPIO pin data (1=high).

// GPIOCON[3:0]: 0 for analog input, 1 for GPIO
enum class GpioConf {
    GPIO0 = 0x01,
    GPIO1 = 0x02,
    GPIO2 = 0x04,
    GPIO3 = 0x08
};
// END


}

// Adc
//
// This interface is based on TI's example code for the ADS124S08 12-channel
// 24-bit sigma-delta ADC. That code can be found at
// github.com/TexasInstruments/precision-adc-examples/tree/main/devices/ads124s08
//
// Only 2 instances of this are needed: one for the Sergeant and one for the
// Soldier. Set the template arguments where those are instantiated according
// to the populated channel selection jumpers on the boards.
template<Pins strt_pin, Pins drdy_pin, Pins cs_pin>
class Adc {
public:
    ///////////////////////
    // GPIO pin mapping. //
    ///////////////////////
    // BEGIN

    // Non-SPI controls. Note that both ADCs share the Reset pin.
    static constexpr Pins RESET {Pins::SYS_RESET};
    static constexpr Pins STRT {strt_pin};
    static constexpr Pins DRDY {drdy_pin};

    // 4-wire SPI controls. Note that both ADCs are on the same SPI bus.
    static constexpr Pins CS {cs_pin};
    static constexpr Pins MOSI {Pins::SPI_MOSI};
    static constexpr Pins MISO {Pins::SPI_MISO};
    static constexpr Pins SCLK {Pins::SPI_SCLK};

    // END

    //////////////////////////////////
    // Constants for the ADS124S08. //
    //////////////////////////////////
    // BEGIN

    static constexpr std::uint8_t NUM_REGISTERS {18};

    // Standard internal clock frequency.
    static constexpr unsigned int FCLK {4096000};
    // ADC resolution in bits.
    static constexpr unsigned int BITRES {24};

    // Command-related data
    static constexpr unsigned int DATA_LENGTH {3};
    static constexpr unsigned int COMMAND_LENGTH {2};
    static constexpr unsigned int STATUS_LENGTH {1};
    static constexpr unsigned int CRC_LENGTH {1};
    static constexpr unsigned int RDATA_COMMAND_LENGTH {1};

    // Conversion-related data
    static constexpr float INT_VREF {2.5f};

    // Fixed timing delays, all in microseconds. These are 64-bit to work with
    // the pico_time high-level API (specifically sleep_us) in the Pico SDK.

    // Min low time for RESET or START
    static constexpr std::uint64_t DELAY_4TCLK {1};
    // Min delay following a RESET before beginning comms
    static constexpr std::uint64_t DELAY_4096TCLK {
        static_cast<std::uint64_t>(4096.0E6 / FCLK)
    };
    // Min delay after device power-up before beginning comms
    static constexpr std::uint64_t DELAY_2p2MS {
        static_cast<std::uint64_t>(0.0022E6)
    };

    // END

    /////////////////////////////////////////////////////
    // Command-related enums and compile-time helpers. //
    /////////////////////////////////////////////////////
    // BEGIN

    // The enums here are described in the above adc_enums namespace.

    using Opcode = adc_enums::Opcode;
    using ReadMode = adc_enums::ReadMode;
    using RegAddr = adc_enums::RegAddr;

    // get_reg_default
    //
    // These are the default values of the registers, and what they are set to
    // on power-up and resets.
    static consteval std::uint8_t get_reg_default(RegAddr reg) {
        switch(reg) {
            case RegAddr::ID:       return 0x00;
            case RegAddr::STATUS:   return 0x80;
            case RegAddr::INPMUX:   return 0x01;
            case RegAddr::PGA:      return 0x00;
            case RegAddr::DATARATE: return 0x14;
            case RegAddr::REF:      return 0x10;
            case RegAddr::IDACMAG:  return 0x00;
            case RegAddr::IDACMUX:  return 0xFF;
            case RegAddr::VBIAS:    return 0x00;
            case RegAddr::SYS:      return 0x10;
            case RegAddr::OFCAL0:   return 0x00;
            case RegAddr::OFCAL1:   return 0x00;
            case RegAddr::OFCAL2:   return 0x00;
            case RegAddr::FSCAL0:   return 0x00;
            case RegAddr::FSCAL1:   return 0x00;
            case RegAddr::FSCAL2:   return 0x40;
            case RegAddr::GPIODAT:  return 0x00;
            case RegAddr::GPIOCON:  return 0x00;
        }
    }

    // END

    ///////////////////////////////////////////
    // Register field description and enums. //
    ///////////////////////////////////////////
    // BEGIN

    // All of these enums are described in the above adc_enums namespace.

    using DevID = adc_enums::DevID;
    using StatusMask = adc_enums::StatusMask;
    using PChannels = adc_enums::PChannels;
    using NChannels = adc_enums::NChannels;
    using DelayMode = adc_enums::DelayMode;
    using PgaCtrl = adc_enums::PgaCtrl;
    using GainMode = adc_enums::GainMode;
    using DatarateMask = adc_enums::DatarateMask;
    using DatarateMode = adc_enums::DatarateMode;
    using RefSetting = adc_enums::RefSetting;
    using IdacmagSetting = adc_enums::IdacmagSetting;
    using Idac2Output = adc_enums::Idac2Output;
    using Idac1Output = adc_enums::Idac1Output;
    using VbiasSetting = adc_enums::VbiasSetting;
    using SysSetting = adc_enums::SysSetting;
    using GpioDir = adc_enums::GpioDir;
    using GpioConf = adc_enums::GpioConf;

    // END

    ///////////////
    // Functions //
    ///////////////
    // BEGIN

    // Default constructor. Does not do anything, since initializing the ADC
    // depends on first initializing the GPIO pins (some of which are shared).
    Adc() = default;

    // ADCs are not copyable or movable.
    Adc(Adc const&) = delete;
    Adc(Adc&&) = delete;
    Adc& operator=(Adc const&) = delete;
    Adc& operator=(Adc&&) = delete;

    // init
    //
    // MUST BE CALLED ONLY AFTER GPIOS AND SPI ARE INITIALIZED.
    // Resets and sets some initial configuration on the ADC.
    // This configuration is specific to the photodiode readout boards. The
    // full list of configurations is rather extensive, but the default choices
    // made here are documented in the implementation header,
    // include/altair/impl/ads124s08_impl.hpp.
    //
    // Returns true if the default register values were successfully applied to
    // the ADC and the ADC reports it is ready.
    // Will not wait if the ADC is nonresponsive and will return false in that
    // case.
    bool init_adc();

    // read_converted_data
    //
    // After DRDY goes low, this can be called to retrieve the data packet.
    // If SENDSTAT in register SYS is set, STATUS will be retrieved as well.
    // If CRC in register SYS is set, the CRC will be retrieved and checked,
    // and this function will return false if the CRC fails.
    //
    // result: The 24-bit signed conversion result will be sign-extended to 32
    // bits and written here.
    //
    // status: If not null and SENDSTAT is set, the STATUS byte will be
    // written here.
    //
    // mode: Direct or Command read modes for how data should be read.
    //
    // Returns false if CRC is enabled and fails, or if result is null. Returns
    // true otherwise.
    bool read_converted_data(
        std::int32_t* result, std::uint8_t* status, ReadMode mode
    ) const;

    // read_single_register
    //
    // Reads the contents of the register at the specified address.
    //
    // address: The address of the specified register to be read.
    //
    // Returns the contents of the read register. Also updates the local
    // register mirror accordingly.
    std::uint8_t read_single_register(RegAddr address);

    // read_multiple_registers
    //
    // Reads the contents of `count` consecutive registers starting at `base`.
    //
    // out: Destination for outputting the contents of the registers. If null,
    // registers are still read to update the local mirror, but they are not
    // output anywhere.
    //
    // base: The address of the first register to be read.
    //
    // count: The number of registers to be read. `base` + `count` must be less
    // than or equal to NUM_REGISTERS.
    //
    // Returns false if base + count is too large, or count is zero. Returns
    // true otherwise.
    bool read_multiple_registers(
        std::uint8_t* out, RegAddr base, std::uint8_t count
    );

    // reset_adc
    //
    // Resets the ADC by sending an SPI command or pulling the RESET pin low
    // depending on the value of `use_pin`. Leaves the ADC with default
    // register values and resets the local register mirror.
    //
    // use_pin: If true, ignores SPI and uses the reset_pin to reset the
    // ADC. Note that this will ALWAYS reset both ADCs as they share a reset
    // line. If false, will use the RESET SPI command, which will only work
    // if the device is not in power-down mode.
    void reset_adc(bool use_pin);

    // send_command
    //
    // Sends a specified SPI command to the ADC. May be used with any command
    // except for WREG, RREG, and RESET, for which the dedicated functions
    // should be used instead.
    //
    // opcode: The opcode for the desired command.
    //
    // Returns true if the opcode is not WREG, RREG, or RESET and the command
    // was successfully sent. Returns false otherwise.
    bool send_command(Opcode opcode) const;

    // start_conversions
    //
    // Begin continuous conversions and wake up the device from power-down by
    // setting STRT high or sending a START command.
    //
    // use_pin: If true, uses the STRT pin rather than SPI. Note that if both
    // boards use the same STRT channel, this will start conversions on both of
    // them.
    void start_conversions(bool use_pin) const;

    // stop_conversions
    //
    // Stops continuous conversions by setting STRT low or sending a STOP
    // command.
    //
    // use_pin: If true, uses the STRT pin rather than SPI. Note that if both
    // boards use the same STRT channel, this will stop conversions on both of
    // them.
    void stop_conversions (bool use_pin) const;

    // write_single_register
    //
    // Writes data to a single register at the specified address. Also updates
    // the local register mirror accordingly.
    //
    // address: The address to write data to.
    //
    // data: The data to write to the register.
    void write_single_register(RegAddr address, std::uint8_t data);

    // write_multiple_registers
    //
    // Writes data to the `count` consecutive registers starting at `base`.
    // Also updates the local register mirror accordingly.
    //
    // base: The first register to write to.
    //
    // data: The array of `count` bytes to write to the registers.
    //
    // count: The number of registers to write to starting at `base`. Note that
    // `base` + `count` must be less than or equal to NUM_REGISTERS.
    //
    // Returns false if data is null, or base + count is too large, or count is
    // zero. Returns true otherwise.
    bool write_multiple_registers(
        RegAddr base, std::uint8_t const* data, std::uint8_t count
    );

    // END

private:
    // Checks the local mirror for register values instead of sending a WREG.
    std::uint8_t get_register_value(RegAddr address) const;

    // Checks the local mirror to see if SENDSTAT is set.
    bool is_sendstat_set() const;

    // Checks the local mirror to see if CRC is set.
    bool is_crc_set() const;

    // Sets the local mirror to the default register values.
    void restore_register_defaults();

    // Computes CRC of an ADC code and compares it to the CRC the ADC sent.
    // Returns false if they do not match.
    // Do not sign-extend the code when passing; leave the upper 8 bits unset.
    static bool check_crc(std::uint32_t code, std::uint8_t adc_crc);

    // The local register mirror.
    std::array<std::uint8_t, NUM_REGISTERS> reg_mirr_ {};
};

// Here is where the channel selection configuration can be altered as needed.

// SgtAdc
//
// The ADS124S08 ADC on the Sergeant board.
using SgtAdc = Adc<Pins::STRT1, Pins::DRDY1, Pins::SPI_CS1>;

// SdrADc
//
// The ADS124S08 ADC on the Soldier board.
using SdrAdc = Adc<Pins::STRT2, Pins::DRDY2, Pins::SPI_CS2>;

} // namespace altair

// This file contains the implementations of the class template functions
// declared here.
#include "altair/impl/ads124s08_impl.hpp"

#endif // ALTAIR_PD_ADS124S08_HPP

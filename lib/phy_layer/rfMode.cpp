/*!
 * @file rfMode.cpp
 * @author Steven Knudsen
 * @date May 25, 2021
 *
 * @details The RF_Mode class that defines the possible modulation modes
 * for the Endurosat UHF Type II radio.
 *
 * @copyright University of Alberta, 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#include "rfMode.hpp"
#include "sdr_driver.h"

namespace ex2 {
  namespace sdr {

    RF_Mode::RF_Mode(RF_ModeNumber rfMode) :
            m_rfMode(rfMode) {
      switch(rfMode) {
        case RF_ModeNumber::RF_MODE_0:
          m_bitRate = 1200;
          m_baudrate_enum = SDR_UHF_1200_BAUD;
          break;
        case RF_ModeNumber::RF_MODE_1:
          m_bitRate = 2400;
          m_baudrate_enum = SDR_UHF_2400_BAUD;
          break;
        case RF_ModeNumber::RF_MODE_2:
          m_bitRate = 4800;
          m_baudrate_enum = SDR_UHF_4800_BAUD;
          break;
        case RF_ModeNumber::RF_MODE_3:
        case RF_ModeNumber::RF_MODE_4:
          m_bitRate = 9600;
          m_baudrate_enum = SDR_UHF_9600_BAUD;
          break;
        case RF_ModeNumber::RF_MODE_5:
        case RF_ModeNumber::RF_MODE_6:
        case RF_ModeNumber::RF_MODE_7:
          m_bitRate = 19200;
          m_baudrate_enum = SDR_UHF_19200_BAUD;
          break;
        default:
          m_bitRate = 9600;
          m_baudrate_enum = SDR_UHF_9600_BAUD;
          break;
      }
    }

    RF_Mode::~RF_Mode() {
    }

  } /* namespace sdr */
} /* namespace ex2 */

sdr_uhf_baud_rate_t get_uhf_baud_t_from_rf_mode_number(uint8_t rf_mode_number)
{
    ex2::sdr::RF_Mode obj((ex2::sdr::RF_Mode::RF_ModeNumber)rf_mode_number);

    return obj.getBaudRateEnum();
}

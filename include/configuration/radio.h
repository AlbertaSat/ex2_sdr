/*!
 * @file radio.h
 * @author Steven Knudsen
 * @date August 20, 2021
 *
 * @details Define constants related to the UHF and S-Band radios
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_CONFIGURATION_RADIO_H_
#define EX2_SDR_CONFIGURATION_RADIO_H_

// UHF Radio
#define UHF_TRANSPARENT_MODE_PREAMBLE_LENGTH           5  // bytes
#define UHF_TRANSPARENT_MODE_SYNC_LENGTH               1  // bytes
#define UHF_TRANSPARENT_MODE_DATA_FIELD_1_LENGTH       1  // byte
#define UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH 128  // bytes
#define UHF_TRANSPARENT_MODE_CRC_LENGTH                2  // bytes

#define UHF_TRANSPARENT_MODE_PACKET_MAX_LENGTH_BYTES \
  ( UHF_TRANSPARENT_MODE_PREAMBLE_LENGTH + \
  UHF_TRANSPARENT_MODE_SYNC_LENGTH + \
  UHF_TRANSPARENT_MODE_DATA_FIELD_1_LENGTH + \
  UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + \
  UHF_TRANSPARENT_MODE_CRC_LENGTH )

#define UHF_TRANSPARENT_MODE_PACKET_LENGTH_BITS (UHF_TRANSPARENT_MODE_PACKET_MAX_LENGTH_BYTES * 8)

#define UHF_TRANSPARENT_MODE_MIN_BAUD          9600
#define UHF_TRANSPARENT_MODE_PACKET_TX_TIME_MS (UHF_TRANSPARENT_MODE_PACKET_LENGTH_BITS * 1000 / UHF_TRANSPARENT_MODE_MIN_BAUD)

// S-band Radio


#endif /* EX2_SDR_CONFIGURATION_RADIO_H_ */

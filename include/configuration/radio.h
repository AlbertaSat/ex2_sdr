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
#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // bytes; UHF transparent mode packet is always 128 bytes
#define UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH 72 // bits
#define UHF_TRANSPARENT_MODE_PACKET_PAYLOAD_LENGTH (UHF_TRANSPARENT_MODE_PACKET_LENGTH - (UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH / 8)) // bits

// S-band Radio


#endif /* EX2_SDR_CONFIGURATION_RADIO_H_ */

/*!
 * @file mac_handler.h
 * @author Steven Knudsen
 * @date Jan 22, 2023
 *
 * @details Methods to aid in the handling sending and receiving data via the MAC.
 * Used to pass buffer data to the MAC for transmission by a target PHY, and to 
 * incrementally receive and reconstruct data received.
 *
 * @copyright AlbertaSat 2023
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_MAC_HANDLER_H_
#define EX2_SDR_MAC_HANDLER_H_

#include "MACWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Process the currently received MPDU and put the results into the 
 * buffer provided
 * 
 * @param[in] my_mac Pointer to the MAC object wrapper
 * @param[in out] mpdu Pointer to the MPDU received via @p get_next_mpdu. 
 * @param[in] mpdu_length The length of the mpdu buffer
*/
int mpdu_to_buffer(mac_t *my_mac, const uint8_t *mpdu, uint8_t **data, int mpdu_length);
    
/*!
 * @brief Check for a new MPDU and return if avaiable.
 * 
 * @param[in] my_mac Pointer to the MAC object wrapper
 * @param[in out] mpdu Pointer to the MPDU received from the PHY via the MAC
 * @param[in] mpdu_length The length of the mpdu buffer
*/
int get_next_mpdu(mac_t *my_mac, void **);

/*!
 * @brief Check for a new MPDU and return if avaiable.
 * 
 * @param[in] my_mac Pointer to the MAC object wrapper
 * @param[in out] mpdu Pointer to the MPDU received from the PHY via the MAC
 * @param[in] mpdu_length The length of the mpdu buffer
*/
int get_mtu_length();

#ifdef __cplusplus
}
#endif
#endif // EX2_SDR_MAC_HANDLER_H_

/*!
 * @file MACWrapper.h
 * @author Steven Knudsen
 * @date Nov. 17, 2021
 *
 * @details Provide a C wrapper for the MAC and its methods
 *
 * @copyright University of Alberta 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#ifndef EX2_SDR_WRAPPER_MAC_H_
#define EX2_SDR_WRAPPER_MAC_H_

#include <stdint.h>
#include <stdbool.h>
#include "error_correctionWrapper.h"
#include "rfModeWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mac;
typedef struct mac mac_t;

/*!
 * @brief C wrapper for MAC object instantiator
 *
 * @param rfMode
 * @param fecScheme
 * @return pointer to @p mac_t that can be used to invoke various MAC methods
 */
mac_t *mac_create(rf_mode_number_t rfMode, error_correction_scheme_t fecScheme);

/*!
 * @brief C wrapper for MAC object destructor
 *
 * @param m A pointer to a @p mac_t that can be NULL
 */
void mac_destroy(mac_t *m);

/*!
 * @brief Return the current FEC scheme
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return The current FEC scheme or if there is a problem, @p ERROR_CORRECTION_SCHEME_BAD_WRAPPER_CONTEXT
 */
error_correction_scheme_t get_error_correction_scheme(mac_t *m);

/*!
 * @brief Set the FEC scheme
 *
 * @param m Pointer to the MAC object wrapper
 * @param fecScheme The new FEC scheme
 *
 * @return true if success, false otherwise
 */
bool set_error_correction_scheme(mac_t *m, error_correction_scheme_t fecScheme);

/*!
 * @brief Return the current RF Mode
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return The current RF Mode or if there is a problem, @p RF_MODE_BAD_WRAPPER_CONTEXT
 */
rf_mode_number_t get_rf_mode_number(mac_t *m);

/*!
 * @brief Set the RF Mode
 *
 * @param m Pointer to the MAC object wrapper
 * @param rf_mode_number The new RF Mode
 *
 * @return true if success, false otherwise
 */
bool set_rf_mode_number (mac_t *m, rf_mode_number_t rf_mode_number);

/************************************************************************/
/* Receive from PHY (radio) methods                                 */
/************************************************************************/

typedef enum {
  PACKET_READY = 0, //(uint16_t) MAC::MAC_PacketProcessingStatus::PACKET_READY,
  PACKET_READY_RESUBMIT_PREVIOUS_PACKET = 1, //(uint16_t) MAC::MAC_PacketProcessingStatus::PACKET_READY_RESUBMIT_PREVIOUS_PACKET,
  READY_FOR_NEXT_PACKET = 2, //(uint16_t) MAC::MAC_PacketProcessingStatus::READY_FOR_NEXT_PACKET
  PACKET_PROCESSING_BAD_WRAPPER_CONTEXT = 100 // needed for wrapper existance checking
} packet_processing_status_t;

/*!
 * @brief Process the received PHY data until a packet is complete.
 *
 * @details Process each PHY data buffer received as an MPDU until a full
 * packet is received or there is an error. The packet length and other
 * details are encapsulated in the MPDU and the MAC processing of each MPDU.
 * 
 * Usually this method is called inside a loop and the return status checked
 * to see when a full packet is available. At that point, a pointer to the
 * raw packet buffer is obtained from @p get_raw_packet_buffer .
 *
 * @param m Pointer to the MAC object wrapper
 * @param payload Pointer to the buffer containing data received from the PHY (i.e., a raw MPDU)
 * @param payload_length The number of payload bytes
 *
 * @return The status of the process operation.
 * If @p PACKET_READY a raw packet is in the raw buffer. 
 * If there is a problem, @p PACKET_PROCESSING_BAD_WRAPPER_CONTEXT is returned.
 */
packet_processing_status_t process_packet(mac_t *m, const uint8_t *payload, const uint32_t payload_length);

/*!
 * @brief When a packet is ready, the raw packet buffer can be retrieved.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return Pointer to the raw packet buffer. If there is a problem, NULL
 * is returned
 */
const uint8_t * get_raw_packet_buffer(mac_t *m);

/*!
 * @brief When a packet is ready, the raw packet buffer length can be returned.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return The raw Packet buffer length in bytes. If there is a problem
 * return -1
 */
int32_t get_raw_packet_length(mac_t *m);


/************************************************************************/
/* Send to PHY (Radio) methods                                      */
/************************************************************************/

/*!
 * @brief Prepare packet for PHY transmission.
 *
 * @details Processed the received packet to create MPDUs ready for
 * transmission by the radio. If the method returns true, then there will
 * be raw MPDUs in the mpdu buffer, which is accessed via @p mpdu_payloads_buffer
 *
 * @param m Pointer to the MAC object wrapper
 * @param packet
 * @param len
 *
 * @return True if the packet was processed, false otherwise
 */
    bool prepare_packet_for_tx(mac_t *m, uint8_t *packet, uint16_t len);

/*!
 * @brief Pointer to MPDU payloads buffer.
 *
 * @details This is needed for the C wrapper accessor
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return pointer to the MPDU payloads buffer.
 */
const uint8_t* mpdu_payloads_buffer(mac_t *m);

/*!
 * @brief The number of bytes in the MPDU payloads buffer.
 *
 * @details The number of bytes will be an integer multiple of the MPDU length.
 * There will be one (1) or more MPDUs in the buffer.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return Number of bytes in the MPDU payloads buffer. If there is a problem
 * returns -1
 */
int32_t mpdu_payloads_buffer_length(mac_t *m);

/*!
 * @brief Return the raw MPDU length
 *
 * @details This should be the length of every transparent mode packet
 *
 * @return Raw MPDU length
 */
uint32_t raw_mpdu_length();

#ifdef __cplusplus
}
#endif

#endif /* EX2_SDR_WRAPPER_MAC_H_ */

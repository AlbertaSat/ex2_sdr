/*!
 * @file MACWrapper.h
 * @author Steven Knudsen
 * @date Nov. 17, 2021
 *
 * @details 
 *
 * @copyright University of Alberta 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#ifndef EX2_SDR_WRAPPER_MAC_H_
#define EX2_SDR_WRAPPER_MAC_H_

//#include "mac.h"
#include "error_correctionWrapper.h"
#include "rfModeWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "csp_types.h"

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
/* Receive from PHY (UHF Radio) methods                                 */
/************************************************************************/

typedef enum {
  CSP_PACKET_READY = 0, //(uint16_t) MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY,
  CSP_PACKET_READY_RESUBMIT_PREVIOUS_PACKET = 1, //(uint16_t) MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY_RESUBMIT_PREVIOUS_PACKET,
  READY_FOR_NEXT_UHF_PACKET = 2, //(uint16_t) MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET
  UHF_PACKET_PROCESSING_BAD_WRAPPER_CONTEXT = 100 // needed for wrapper existance checking
} uhf_packet_processing_status_t;

/*!
 * @brief Process the received UHF data as an MPDU.
 *
 * @details Process each MPDU received (UHF received data in transparent
 * mode) until a full CSP packet is received or there is an error.
 *
 * @param m Pointer to the MAC object wrapper
 * @param uhf_payload The transparent mode data received from the UHF radio
 * @param payload_length The number of transparent mode data bytes received
 *
 * @return The status of the process operation. If @p CSP_PACKET_READY,
 * a raw CSP packet (i.e., a @p csp_packet_t cast to a uint8_t pointer)
 * is in the raw buffer. If there is a problem, @p UHF_PACKET_PROCESSING_BAD_WRAPPER_CONTEXT
 * is returned.
 */
uhf_packet_processing_status_t process_uhf_packet(mac_t *m, const uint8_t *uhf_payload, const uint32_t payload_length);

/*!
 * @brief When ready, the raw CSP Packet buffer can be retrieved.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return Pointer to the raw CSP packet buffer. If there is a problem, NULL
 * is returned
 */
const uint8_t * get_raw_csp_packet_buffer(mac_t *m);

/*!
 * @brief When ready, the raw CSP Packet buffer length can be returned.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return The raw CSP Packet buffer length in bytes. If there is a problem
 * return -1
 */
int32_t get_raw_csp_packet_buffer_length(mac_t *m);

/*!
 * @brief When ready, the length of the CSP packet data in the raw CSP
 * Packet buffer can be returned.
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return The length of the CSP packet data in the raw CSP Packet buffer. If
 * there is a problem return -1
 */
int32_t get_raw_csp_packet_length(mac_t *m);


/************************************************************************/
/* Send to PHY (UHF Radio) methods                                      */
/************************************************************************/

/*!
 * @brief Receive and encode new CSP packet.
 *
 * @details Processed the received CSP packet to create MPDUs ready for
 * transmission by the UHF radio in transparent mode. If the method returns
 * true, then there will be raw MPDUs in the mpdu payloads buffer
 *
 * @param m Pointer to the MAC object wrapper
 * @param cspPacket
 *
 * @return True if the CSP packet was encoded, false otherwise
 */
bool receive_csp_packet(mac_t *m, csp_packet_t * csp_packet);

/*!
 * @brief Pointer to MPDU payloads buffer.
 *
 * @details This is needed for the C wrapper accessor
 *
 * @param m Pointer to the MAC object wrapper
 *
 * @return pointer to the MPDU payloads buffer.
 */
const uint8_t * mpdu_payloads_buffer(mac_t *m);

/*!
 * @breif The number of bytes in the MPDU payloads buffer.
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

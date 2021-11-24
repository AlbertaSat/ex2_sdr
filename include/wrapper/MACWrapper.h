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
//mac_t *mac_create();

/*!
 * @brief C wrapper for MAC object destructor
 *
 * @param m A pointer to a @p mac_t that can be NULL
 */
void mac_destroy(mac_t *m);

// ErrorCorrection::ErrorCorrectionScheme getErrorCorrectionScheme ();
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

// MAC_UHFPacketProcessingStatus processUHFPacket(const uint8_t *uhfPayload, const uint32_t payloadLength);
uhf_packet_processing_status_t process_uhf_packet(mac_t *m, const uint8_t *uhf_payload, const uint32_t payload_length);

// const uint8_t * getRawCspPacketBuffer ()

// uint32_t getRawCspPacketBufferLength ()

// uint32_t getRawCspPacketLength ()

/************************************************************************/
/* Send to PHY (UHF Radio) methods                                      */
/************************************************************************/

// bool receiveCSPPacket(csp_packet_t * cspPacket);
//bool receive_CSP_packet(csp_packet_t * csp_packet);

// uint8_t * mpduPayloadsBuffer();

// uint32_t mpduPayloadsBufferLength() const;



#ifdef __cplusplus
}
#endif

#endif /* EX2_SDR_WRAPPER_MAC_H_ */

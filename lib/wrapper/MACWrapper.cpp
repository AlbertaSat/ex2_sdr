/*!
 * @file MACWrapper.c
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
#include <stdlib.h>
#include "MACWrapper.h"

#include "mac.hpp"
#include "mpdu.hpp"


struct mac {
  void *obj;
};

mac_t *mac_create(rf_mode_number_t rfMode, error_correction_scheme_t fecScheme)
{
  mac_t *m;
  ex2::sdr::MAC *obj;

  m      = (typeof(m))malloc(sizeof(*m));
  obj    = new ex2::sdr::MAC((ex2::sdr::RF_Mode::RF_ModeNumber) rfMode, (ex2::sdr::ErrorCorrection::ErrorCorrectionScheme) fecScheme);
  m->obj = obj;

  return m;
}

void mac_destroy(mac_t *m)
{
  if (m == NULL)
    return;

  delete static_cast<ex2::sdr::MAC *>(m->obj);
  free(m);
  m = NULL; // @todo not sure this really does anything that matters
}

// ErrorCorrection::ErrorCorrectionScheme getErrorCorrectionScheme ();
error_correction_scheme_t get_error_correction_scheme(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return ERROR_CORRECTION_SCHEME_BAD_WRAPPER_CONTEXT;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return (error_correction_scheme_t) (obj->getErrorCorrectionScheme());
}

bool set_error_correction_scheme(mac_t *m, error_correction_scheme_t fecScheme) {
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return false;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  obj->setErrorCorrectionScheme((ex2::sdr::ErrorCorrection::ErrorCorrectionScheme) fecScheme);

  return true;
}

rf_mode_number_t get_rf_mode_number(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return RF_MODE_BAD_WRAPPER_CONTEXT;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return (rf_mode_number_t) (obj->getRFModeNumber());
}

bool set_rf_mode_number (mac_t *m, rf_mode_number_t rf_mode_number)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return false;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  obj->setRFModeNumber((ex2::sdr::RF_Mode::RF_ModeNumber) rf_mode_number);

  return true;
}

uhf_packet_processing_status_t process_uhf_packet(mac_t *m, const uint8_t *uhf_payload, const uint32_t payload_length)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return UHF_PACKET_PROCESSING_BAD_WRAPPER_CONTEXT;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return (uhf_packet_processing_status_t) (obj->processUHFPacket(uhf_payload, payload_length));
}

const uint8_t * get_raw_csp_packet_buffer(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return NULL;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->getRawCspPacketBuffer();
}

int32_t get_raw_csp_packet_buffer_length(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return -1;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->getRawCspPacketBufferLength();
}

int32_t get_raw_csp_packet_length(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return 1;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->getRawCspPacketLength();
}

bool receive_csp_packet(mac_t *m, csp_packet_t * csp_packet)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return false;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->receiveCSPPacket(csp_packet);
}

const uint8_t * mpdu_payloads_buffer(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return NULL;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->mpduPayloadsBuffer();
}

int32_t mpdu_payloads_buffer_length(mac_t *m)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return -1;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return obj->mpduPayloadsBufferLength();
}

uint32_t raw_mpdu_length()
{
  return ex2::sdr::MPDU::rawMPDULength();
}


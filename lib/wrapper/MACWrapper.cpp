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

//using namespace ex2::sdr;

struct mac {
  void *obj;
};

//mac_t *mac_create()
//{
//  mac_t *m;
//  ex2::sdr::MAC *obj;
//
//  m      = (typeof(m))malloc(sizeof(*m));
//  obj    = new ex2::sdr::MAC(1);
////  obj    = new MAC((RF_Mode::RF_ModeNumber) rfMode, (ErrorCorrection::ErrorCorrectionScheme) fecScheme);
//  m->obj = obj;
//
//  return m;
//}

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
  printf("m %ld\n",m);

  if (m == NULL)
    return;
  printf("m deleting\n");
  printf("m->obj %ld\n",m->obj);

  delete static_cast<ex2::sdr::MAC *>(m->obj);
  printf("m->obj %ld\n",m->obj);
  free(m);
  m = __null;
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


//void mtest_add(mac_t *m, int val)
//{
//  ex2::sdr::MAC *obj;
//
//  if (m == NULL)
//    return;
//
//  obj = static_cast<ex2::sdr::MAC *>(m->obj);
//  obj->add(val);
//}

uhf_packet_processing_status_t process_uhf_packet(mac_t *m, const uint8_t *uhf_payload, const uint32_t payload_length)
{
  ex2::sdr::MAC *obj;

  if (m == NULL)
    return UHF_PACKET_PROCESSING_BAD_WRAPPER_CONTEXT;

  obj = static_cast<ex2::sdr::MAC *>(m->obj);
  return (uhf_packet_processing_status_t) (obj->processUHFPacket(uhf_payload, payload_length));
}

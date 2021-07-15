/*!
 * @file NoFEC.cpp
 * @author Sknud
 * @date Jul. 14, 2021
 *
 * @details 
 *
 * @copyright 
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "NoFEC.hpp"

namespace ex2 {
  namespace sdr {

    //      NoFEC::NoFEC(ErrorCorrection::ErrorCorrectionScheme ecScheme) : FEC(ecScheme) {
    //      }

    NoFEC::~NoFEC() {  }

    PPDU_u8
    NoFEC::encode(PPDU_u8 &payload) {
      return payload;
    }

    uint32_t
    NoFEC::decode(PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate;
      decodedPayload.resize(0);
      decodedPayload = encodedPayload;
//      uint32_t i;
//      for (i = 0; i < encodedPayload.size(); i++) {
//        decodedPayload.push_back(encodedPayload[i]);
//      }

      return 0;
    }


  } /* namespace sdr */
} /* namespace ex2 */

/*!
 * @file NoFEC.cpp
 * @author Sknud
 * @date Jul. 14, 2021
 *
 * @details The "No FEC" scheme just copies the input vectors to outputs.
 * No FEC operations are performed, so there is no error correction ability.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "NoFEC.hpp"

namespace ex2 {
  namespace sdr {

    NoFEC::~NoFEC() {  }

    PPDU_u8
    NoFEC::encode(PPDU_u8 &payload) {
      return payload;
    }

    uint32_t
    NoFEC::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate; // Not used in this method

      decodedPayload.resize(0); // Resize in all FEC decode methods

      // Here is where we apply the FEC decode algorithm.
      // For no FEC, just copy the data
      decodedPayload = encodedPayload;

      return 0;
    }

  } /* namespace sdr */
} /* namespace ex2 */

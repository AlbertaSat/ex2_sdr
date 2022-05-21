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

    std::vector<uint8_t>
    NoFEC::encode(const std::vector<uint8_t>& payload) {
      std::vector<uint8_t> notEncoded = payload;
      return notEncoded;
    }

    uint32_t
    NoFEC::decode(std::vector<uint8_t> &encodedPayload, float snrEstimate,
      std::vector<uint8_t> &decodedPayload) {

      (void) snrEstimate; // Not used in this method

      decodedPayload.resize(0); // Resize in all FEC decode methods

      // Here is where we apply the FEC decode algorithm.
      // For no FEC, just copy the data
      decodedPayload = encodedPayload;

      return 0;
    }

  } /* namespace sdr */
} /* namespace ex2 */

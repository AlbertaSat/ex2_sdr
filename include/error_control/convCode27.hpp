/*!
 * @file convCode27.hpp
 * @author Arash Yazdani
 * @date July 21, 2021
 *
 * @details Convolution Code with rate = 1/2
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_CONVCODE27_H_
#define EX2_SDR_ERROR_CONTROL_CONVCODE27_H_

#include "FEC.hpp"

namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class convCode27 : public FEC {
    public:

      convCode27(ErrorCorrection::ErrorCorrectionScheme ecScheme) : FEC(ecScheme) { }

      ~convCode27();

      PPDU_u8 encode(PPDU_u8 &payload);

      uint32_t decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
        PPDU_u8::payload_t& decodedPayload);

    private:
      const double rate = 1/2;// can be read from FEC method
      const uint8_t constraint_length = 7; // aka K
      uint8_t adder(uint8_t * payload_sym , std::vector<uint8_t> g);
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_CONVCODE27_H_ */

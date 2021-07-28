/*!
 * @file convCodeR_1_2.hpp
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

#ifndef EX2_SDR_ERROR_CONTROL_CONVCODER_1_2_H_
#define EX2_SDR_ERROR_CONTROL_CONVCODER_1_2_H_

#include "FEC.hpp"

namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class convCodeR_1_2 : public FEC {
    public:

      convCodeR_1_2(ErrorCorrection::ErrorCorrectionScheme ecScheme) : FEC(ecScheme) { }

      ~convCodeR_1_2();

      PPDU_u8 encode(PPDU_u8 &payload);

      uint32_t decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
        PPDU_u8::payload_t& decodedPayload);

    private:
      const double rate = 1/2;// can be read from FEC method
      const uint8_t constraint_length = 7; // aka K
      uint8_t adder(uint8_t payload_sym , std::vector<uint8_t> g);
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_CONVCODER_1_2_H_ */

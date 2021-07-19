/*!
 * @file FEC.hpp
 * @author StevenKnudsen
 * @date June 21, 2021
 *
 * @details A Forward Error Correction factory that creates instances of various
 * FEC codecs.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_FEC_H_
#define EX2_SDR_ERROR_CONTROL_FEC_H_

#include <cstdint>
#include <string>
#include <vector>

#include "error_correction.hpp"
#include "ppdu_u8.hpp"

namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class FEC {
    public:
      static FEC *makeFECCodec(ErrorCorrection::ErrorCorrectionScheme ecScheme);

      FEC(ErrorCorrection::ErrorCorrectionScheme ecScheme);

      virtual ~FEC() {}

      /*!
       * @brief A virtual function to encode a payload using the FEC scheme
       *
       * @param[in] payload The payload to encode
       * @return The encoded payload
       */
      virtual PPDU_u8 encode(PPDU_u8 &payload) = 0;

      /*!
       * @brief A virtual function to decode a payload using the FEC scheme
       *
       * @param[in] encodedPayload The encoded payload
       * @param[in] snrEstimate An estimate of the SNR for FEC schemes that need it.
       * @param[out] decodedPayload The resulting decoded payload
       * @return The number of bit errors from the decoding process
       */
      virtual uint32_t decode(const PPDU_u8::payload_t &encodedPayload, float snrEstimate,
        PPDU_u8::payload_t &decodedPayload) = 0;

    private:
      ErrorCorrection::ErrorCorrectionScheme m_ecScheme;
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_FEC_H_ */

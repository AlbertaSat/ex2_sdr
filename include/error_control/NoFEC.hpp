/*!
 * @file NoFEC.hpp
 * @author StevenKnudsen
 * @date July 12, 2021
 *
 * @details A passthrough FEC codec.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_NOFEC_H_
#define EX2_SDR_ERROR_CONTROL_NOFEC_H_

#include "FEC.hpp"

namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class NoFEC : public FEC {
    public:

      NoFEC(ErrorCorrection::ErrorCorrectionScheme ecScheme) : FEC(ecScheme) { }

      ~NoFEC();

      PPDU_u8 encode(PPDU_u8 &payload);

      uint32_t decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
        PPDU_u8::payload_t& decodedPayload);

    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_NOFEC_H_ */

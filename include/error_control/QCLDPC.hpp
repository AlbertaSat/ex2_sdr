/*!
 * @file QCLDPC.hpp
 * @author StevenKnudsen
 * @date Sept 27, 2021
 *
 * @details A passthrough FEC codec.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_QCLDPC_H_
#define EX2_SDR_ERROR_CONTROL_QCLDPC_H_

#include <stdexcept>

#include "FEC.hpp"

namespace ex2 {
  namespace sdr {

    class QCLDPCException: public std::runtime_error {

    public:
      QCLDPCException(const std::string& message);
    };
    /*!
     * @brief Define a forward error correction scheme.
     */
    class QCLDPC : public FEC {
    public:

      QCLDPC(ErrorCorrection::ErrorCorrectionScheme ecScheme);

      ~QCLDPC();

      PPDU_u8 encode(PPDU_u8 &payload);

      uint32_t decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
        PPDU_u8::payload_t& decodedPayload);

    private:
      ErrorCorrection *m_errorCorrection;
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_QCLDPC_H_ */

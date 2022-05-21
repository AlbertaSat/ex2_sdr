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

    /*!
     * @brief Define a forward error correction scheme.
     */
    class QCLDPC : public FEC {
    public:

      QCLDPC(ErrorCorrection::ErrorCorrectionScheme ecScheme);

      ~QCLDPC();

      std::vector<uint8_t> encode(const std::vector<uint8_t>& payload);

      uint32_t decode(std::vector<uint8_t>& encodedPayload, float snrEstimate,
        std::vector<uint8_t>& decodedPayload);

    private:
      ErrorCorrection *m_errorCorrection = 0;
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_QCLDPC_H_ */

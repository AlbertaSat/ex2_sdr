/*!
 * @file ConvolutionalCodecHD.hpp
 * @author Steven Knudsen
 * @date Dec 1, 2021
 *
 * @details The Convolutional Codec provides convolutional encoding and
 * hard-decision decoding for the CCSDS schemes defined in @p error_correction.hpp
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_CONVOLUTIONAL_CODEC_H_
#define EX2_SDR_ERROR_CONTROL_CONVOLUTIONAL_CODEC_H_

#include "FEC.hpp"

#include "viterbi.hpp"


namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class ConvolutionalCodecHD : public FEC {
    public:

      ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme ecScheme);

      ~ConvolutionalCodecHD();

      PPDU_u8 encode(PPDU_u8 &payload);

      uint32_t decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
        PPDU_u8::payload_t& decodedPayload);

    private:
      ErrorCorrection *m_errorCorrection = 0;
      ViterbiCodec *m_codec = 0;
    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_CONVOLUTIONAL_CODEC_H_ */

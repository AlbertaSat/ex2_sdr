/*!
 * @file ConvolutionalCodecHD.cpp
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

#include "ConvolutionalCodecHD.hpp"
#include "mpdu.hpp"
#include "mpduUtility.hpp"

#define CC_HD_DEBUG 0

// CCSDS polynomials and constraint length; see CCSDS 131.0-B-3
#define CCSDS_CONVOLUTIONAL_CODE_CONSTRAINT 7
#define CCSDS_CONVOLUTIONAL_CODE_POLY_G1 121 // 0x79 0b1111001
#define CCSDS_CONVOLUTIONAL_CODE_POLY_G2 91  // 0x5B 0b1011011

namespace ex2 {
  namespace sdr {

    ConvolutionalCodecHD::ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme ecScheme)  : FEC(ecScheme) {


      // Only the CCSDS schemes are permitted
      switch (ecScheme) {
        case ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2:
          break;
        case ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3:
          throw new FECException("Convolutional coding rate 2/3 not yet implemented");
          break;
        case ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4:
          throw new FECException("Convolutional coding rate 3/4 not yet implemented");
          break;
        case ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6:
          throw new FECException("Convolutional coding rate 5/6 not yet implemented");
          break;
        case ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8:
          throw new FECException("Convolutional coding rate 7/8 not yet implemented");
          break;
        default:
          throw new FECException("Must be a Convolutional Codec scheme.");
          break;
      }

      // @TODO does this belong in the FEC constructor?
      m_errorCorrection = new ErrorCorrection(ecScheme, (MPDU::maxMTU() * 8));
//      std::vector<int> polynomials{ CCSDS_CONVOLUTIONAL_CODE_POLY_G1, CCSDS_CONVOLUTIONAL_CODE_POLY_G2};
//
//      m_codec = new ViterbiCodec(CCSDS_CONVOLUTIONAL_CODE_CONSTRAINT, polynomials);

      // for dev of memory-reduced algorithm, use constraint len 3 and { 7, 5 }, which is used for a unit test; see qa_viterbi
      std::vector<int> polynomials{ 7, 5};

      m_codec = new ViterbiCodec(3, polynomials);
    }

    ConvolutionalCodecHD::~ConvolutionalCodecHD() {
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
      if (m_codec != NULL) {
        delete m_codec;
      }
    }

    std::vector<uint8_t>
    ConvolutionalCodecHD::encode(const std::vector<uint8_t>& payload)
    {
      // Encode the 8 BPS message
      std::vector<uint8_t> encodedPayload = m_codec->encodePacked(payload);

      return encodedPayload;
    }

    uint32_t
    ConvolutionalCodecHD::decode(std::vector<uint8_t>& encodedPayload, float snrEstimate,
      std::vector<uint8_t>& decodedPayload) {

      (void) snrEstimate; // Not used in this method

      decodedPayload.resize(0); // Resize in all FEC decode methods

      if (!m_codec) {
        // make it very obviously fail by returning a huge number of bit errors
        return UINT32_MAX;
      }
      else {
        // assume the encoded payload is packed, 8 bits per byte. Repack to be
        // 1 bit per byte
        MPDUUtility::repack(encodedPayload, MPDUUtility::BPSymb_8, MPDUUtility::BPSymb_1);

        // Decode the 1 bit per byte payload.
        ViterbiCodec::bitarr_t decoded = m_codec->decode(encodedPayload);

        // Repack the result to be 8 bits per byte
        MPDUUtility::repack(decoded, MPDUUtility::BPSymb_1, MPDUUtility::BPSymb_8);
        decodedPayload.resize(0);
        decodedPayload.insert(decodedPayload.end(),decoded.begin(),decoded.end());

        // We have no way to know if there are bit errors, so return zero (0)
        return 0;
      }
    }

  } /* namespace sdr */
} /* namespace ex2 */

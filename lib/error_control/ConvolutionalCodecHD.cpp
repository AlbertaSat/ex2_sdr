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

#define CC_HD_DEBUG 0

// CCSDS polynomials and constraint length; see CCSDS 131.0-B-3
#define CCSDS_CONVOLUTIONAL_CODE_CONSTRAINT 7
#define CCSDS_CONVOLUTIONAL_CODE_POLY_G1 121 // 0x79 0b1111001
#define CCSDS_CONVOLUTIONAL_CODE_POLY_G2 91  // 0x5B 0b1011011

namespace ex2 {
  namespace sdr {

    ConvolutionalCodecHD::ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme ecScheme)  : FEC(ecScheme) {

      // @TODO does this belong in the FEC constructor?
      m_errorCorrection = new ErrorCorrection(ecScheme, (MPDU::maxMTU() * 8));

      // Only the CCSDS schemes are permitted
      switch (this->m_errorCorrection->getErrorCorrectionScheme()) {
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
//      m_errorCorrection = new ErrorCorrection(ecScheme, (MPDU::maxMTU() * 8));
      std::vector<int> polynomials{ CCSDS_CONVOLUTIONAL_CODE_POLY_G1, CCSDS_CONVOLUTIONAL_CODE_POLY_G2};

      m_codec = new ViterbiCodec(CCSDS_CONVOLUTIONAL_CODE_CONSTRAINT, polynomials);
    }

    ConvolutionalCodecHD::~ConvolutionalCodecHD() {
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
      if (m_codec != NULL) {
        delete m_codec;
      }
    }

    PPDU_u8
    ConvolutionalCodecHD::encode(PPDU_u8 &payload) {

      if (!m_codec) {
        PPDU_u8 notEncoded(PPDU_u8::BPSymb_8);
        notEncoded.clearPayload();

        return notEncoded;
      }
      else {
        // Don't assume the input has 1 bit per symbol (aka uint8_t)
        if (payload.getBps() != PPDU_u8::BPSymb_1) {
          payload.repack(PPDU_u8::BPSymb_1);
        }

        // Encode the 1 BPS message
        ViterbiCodec::bitarr_t bitPayload = payload.getPayload();
        ViterbiCodec::bitarr_t encodedPayload = m_codec->encode(bitPayload);

#if CC_HD_DEBUG
       printf("encode input length %ld encoded length %ld\n", bitPayload.size(), encodedPayload.size());
#endif

       // Convert the codeword to a PPDU_u8 at 8 BPS (packed)
        PPDU_u8 encodedPDU(encodedPayload,PPDU_u8::BPSymb_1);
        encodedPDU.repack(PPDU_u8::BPSymb_8);
        payload.repack(PPDU_u8::BPSymb_8);

        return encodedPDU;
      }
    }

    uint32_t
    ConvolutionalCodecHD::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate; // Not used in this method

      decodedPayload.resize(0); // Resize in all FEC decode methods

      if (!m_codec) {
        // make it very obviously fail by returning a huge number of bit errors
        return UINT32_MAX;
      }
      else {
        // assume the encoded payload is packed, 8 bits per byte. Repack to be
        // 1 bit per byte
        PPDU_u8 ePPDU(encodedPayload, PPDU_u8::BPSymb_8);
        ePPDU.repack(PPDU_u8::BPSymb_1);
        PPDU_u8::payload_t ePPDUpayload = ePPDU.getPayload();

//        // @todo generalize this for rates in addition to 1/2
//        // Check the codeword is the length expected. It may have been zero-
//        // padded in the encode method
//        if (m_errorCorrection->getMessageLen()*2 < ePPDUpayload.size()) {
//          printf("codeword len %ld should be %ld\n",ePPDUpayload.size(),m_errorCorrection->getMessageLen()*2);
//          printf("resize codeword please\n");
//          uint32_t newSize = m_errorCorrection->getMessageLen()*2;
//          ePPDUpayload.resize(newSize);
//          printf("codeword len %ld should be %ld\n",ePPDUpayload.size(),m_errorCorrection->getMessageLen()*2);
//        }

        // Decode the 1 bit per byte payload.
        ViterbiCodec::bitarr_t dPPDUpayload = m_codec->decode(ePPDUpayload);

        // Repack the result to be 8 bits per byte
        PPDU_u8 dPPDU(dPPDUpayload, PPDU_u8::BPSymb_1);
        dPPDU.repack(PPDU_u8::BPSymb_8);
        dPPDUpayload = dPPDU.getPayload();
//        printf("decoded payload size %ld\n",dPPDUpayload.size());
        decodedPayload.insert(decodedPayload.end(),dPPDUpayload.begin(),dPPDUpayload.end());

        // We have no way to know if there are bit errors, so return zero (0)
        return 0;
      }
    }

  } /* namespace sdr */
} /* namespace ex2 */

/*!
 * @file qa_viterbit.cpp
 * @author Steven Knudsen
 * @date Dec 15, 2021
 *
 * @details Unit test for the viterbi class as written by vitalsong.
 *
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include "ConvolutionalCodecHD.hpp"
#include "mpdu.hpp"
#include "ppdu_f.hpp"
#include "vectorTools.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "csp.h"
#include "csp_types.h"
#include "csp_buffer.h"

#ifdef __cplusplus
}
#endif

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_CC_HD_DEBUG 0 // set to 1 for debugging output

uint8_t numOnesInByte(uint8_t b) {
  uint8_t count = (b >> 7) & 0x01;
  count += (b >> 6) & 0x01;
  count += (b >> 5) & 0x01;
  count += (b >> 4) & 0x01;
  count += (b >> 3) & 0x01;
  count += (b >> 2) & 0x01;
  count += (b >> 1) & 0x01;
  count += b & 0x01;

  return count;
}
/*!
 * @brief Check FEC decoding for the scheme provided.
 *
 * @details Using randomly generated messages at the specificy SNR, test if the
 * specified bit-error rate can be achieved.
 *
 * @param errorCorrectionScheme The error correction scheme.
 * @param snr The signal to noise ratio of the generated data in the range [-20,60]
 * @param ber The target bit-error rate in the range [1e-6,0.1]
 * @param berExceedExpected If true, the target @p ber is expected to be exceeded
 * @param berTolerance The percent tolerance to allow for the BER to be exceeded, in the range [0,100]
 */
void
check_decoder_ber (
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme,
  double snr,
  double ber,
  bool berExceedExpected,
  double berTolerance)
{
  if (ber > 0.1f or ber < 1e-7) {
    printf("The BER must be in the range [1e-7,0.1]\n");
    throw std::exception();
  }

  if (snr > 60.0f or snr < -20.0f) {
    printf("The SNR must be in the range [-20,60]\n");
    throw std::exception();
  }

  if (berExceedExpected) {
    if (berTolerance < 0.0f or berTolerance > 100.0f) {
      printf("The berTolerance must be in the range [0,100]\n");
      throw std::exception();
    }
  }

  ConvolutionalCodecHD ccHDCodec(errorCorrectionScheme);

  ErrorCorrection ec(errorCorrectionScheme, MPDU::maxMTU()*8);

  // Get ready for simulating noise at the input SNR
  float sigma2 = 0.5 / pow (10.0, snr / 10.0); // noise variance
  boost::mt19937 *rng = new boost::mt19937 ();
  rng->seed (time (NULL));
  boost::normal_distribution<> distribution (-sqrt(sigma2), sqrt(sigma2));
  boost::variate_generator<boost::mt19937, boost::normal_distribution<> > dist (*rng, distribution);

  // Deal with data in unpacked format, i.e., 1 bit per byte.
  // The message length is returned in bits. It's easiest to handle the data as
  // being 1 bit per 8-bit symbol, aka unpacked
  unsigned int payloadBitCount = ec.getMessageLen();
  unsigned int payloadByteCount = payloadBitCount/8;
  PPDU_u8::payload_t packedMessage(payloadByteCount);

  // Calculate the number of bits needed for the desired BER. Assume at least
  // 100 errors are needed to establish the BER.
  uint32_t numBitsForBER = 100 / ber;
  uint32_t numBits = 0;
  uint32_t numErrors = 0;

  std::srand(std::time(0));

  while (numBits < numBitsForBER && numErrors < 100)
  {
    // Make a random data payload
    for (unsigned int i = 0; i < payloadByteCount; i++) {
      packedMessage[i] = (uint8_t) (std::rand () & 0x00FF);
    }

#if QA_CC_HD_DEBUG
    printf("message  : ");
    for (int i = 0; i < 10; i++) {
      printf("0x%02x ", packedMessage[i]);
    }
    printf("\n");
#endif
    PPDU_u8 dataPPDU (packedMessage, PPDU_u8::BitsPerSymbol::BPSymb_8);

    // Encode the packet
    PPDU_u8 encodedPPDU = ccHDCodec.encode (dataPPDU);
    PPDU_u8::payload_t payload = encodedPPDU.getPayload ();
#if QA_CC_HD_DEBUG
    printf("codeword : ");
    for (int i = 0; i < 10; i++) {
      printf("0x%02x ", payload[i]);
    }
    printf("\n");
#endif

    PPDU_f::payload_t payloadFloat;
    VectorTools::bytesToFloat(payload, true, false, true, 1.0f, payloadFloat);

    // Add noise. The bytesToFloat method makes float symbols of mag 1.
#if QA_CC_HD_DEBUG
    float pSignal = 0.0f;
    float pNoise = 0.0f;
#endif
    float noise;
    for (uint32_t i = 0; i < payloadFloat.size(); i++) {
      noise = dist();
#if QA_CC_HD_DEBUG
      pNoise += noise*noise;
      pSignal += payloadFloat[i]*payloadFloat[i];
#endif
      payloadFloat[i] += noise;
    }

#if QA_CC_HD_DEBUG
    pSignal /= (float) payloadFloat.size();
    pNoise /= (float) payloadFloat.size();
    printf("pSignal = %g pNoise = %g snr = %g\n", pSignal, pNoise, 10.0f*std::log10(pSignal/pNoise));
#endif
    // We convert back to binary data and impose our own hard decision.
    PPDU_u8::payload_t payloadPlusNoise;
    float threshold = 0.0; // The float payload was NRZ, so in [-1,1]
    VectorTools::floatToBytes(threshold, false, payloadFloat, payloadPlusNoise);

#if QA_CC_HD_DEBUG
    printf("cw+noise : ");
    for (unsigned int i = 0; i < 10; i++) {
      printf("0x%02x ", payloadPlusNoise[i]);
    }
    printf("\n");
#endif

    // Try to decode the noisy codeword
    PPDU_u8::payload_t decodedMessage;
    ccHDCodec.decode(payloadPlusNoise, snr, decodedMessage);
#if QA_CC_HD_DEBUG
    printf("dmessage : ");
    for (int i = 0; i < 10; i++) {
      printf("0x%02x ", decodedMessage[i]);
    }
#endif
    // count the bit errors in the decoded message
    uint8_t diffByte;
    for (unsigned int i = 0; i < packedMessage.size(); i++) {
      diffByte = packedMessage[i] ^ decodedMessage[i];
      numErrors += numOnesInByte(diffByte);
    }
#if QA_CC_HD_DEBUG
    printf("At %g dB SNR, numErrors = %d\n", snr, numErrors);
    printf("\n--------------------\n");
#endif
    numBits += payloadBitCount;
  } // while not enough bits for BER
  printf("@%g dB, numbBits %d numErrors %d\n", snr, numBits, numErrors);

  std::string ecn = ec.ErrorCorrectionName(errorCorrectionScheme);
  double calcBER = (double) numErrors / (double) numBits;
  if (berExceedExpected) {
    if (calcBER < ber) {
      ASSERT_TRUE(calcBER > (1.0 - berTolerance/100.0)*ber) << (boost::format ("BER too low for %1% @ %2% dB SNR") % ecn % snr).str();
    }
  }
  else {
    if (calcBER > ber) {
      ASSERT_TRUE(calcBER > (1.0 + berTolerance/100.0)*ber) << (boost::format ("BER too high for %1% @ %2% dB SNR") % ecn % snr).str();
    }
  }
} // check decoder


/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(convolutional_codec_hd, constructor_accessor )
{
  /* ----------------------------------------------------------------------
   * Confirm rate 2/3, 3/4, 5/6, and 7/8 CCSDS cannot be instantiated.
   * Confirm rate 1/2 CCSDS can be instantiated.
   * ----------------------------------------------------------------------
   */
  ConvolutionalCodecHD *ccHDCodec;

  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3);
    FAIL() << "Should not be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_2_3.";
  }
  catch (FECException *e) {
  }
  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4);
    FAIL() << "Should not be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_3_4.";
  }
  catch (FECException *e) {
  }
  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6);
    FAIL() << "Should not be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_5_6.";
  }
  catch (FECException *e) {
  }
  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8);
    FAIL() << "Should not be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_7_8.";
  }
  catch (FECException *e) {
  }

  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2);
  }
  catch (FECException *e) {
    FAIL() << "Should be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_1_2.";
  }

  if (ccHDCodec) {
    // Other than the encode and decode methods, there are no other methods to test.
  }
}

TEST(convolutional_codec_hd, r_1_2_simple_encode_decode_no_errs )
{
  /* ----------------------------------------------------------------------
   * Create rate 1/2 CCSDS codec and test encode and decode with no errors
   *
   * The codec should not care what length of message to encode and decode.
   * For the UHF radio we have a max payload of 119 bytes, but we should test
   * it, longer, and shorter messages. Might as well follow the CSP packet
   * lengths used in MAC testing understanding that this unit test is
   * independent of the MAC fragmentation into UHF payloads...
   *
   * ----------------------------------------------------------------------
   */
  ConvolutionalCodecHD *ccHDCodec;

  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2);

    // Do a little CSP config work
    csp_conf_t cspConf;
    csp_conf_get_defaults(&cspConf);
    cspConf.buffer_data_size = 4096; // TODO set as CSP_MTU
    csp_init(&cspConf);


    // Set the CSP packet test lengths so that
    // * a zero length packet is tested
    // * a non-zero length packet fits well into one MPDU
    // * a non-zero length a packet just fits into one MPDU
    // * a non-zero length a packet needs more than one MPDU
    // * the max size packet
    uint16_t const numCSPPackets = 5;
    uint16_t cspPacketDataLengths[numCSPPackets] = {0, 10, 103, 358, 4095};
    for (uint16_t currentCSPPacket = 0; currentCSPPacket < numCSPPackets; currentCSPPacket++) {

      csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currentCSPPacket]);

      if (packet == NULL) {
        // Could not get buffer element
        csp_log_error("Failed to get CSP buffer");
        FAIL() << "Failed to get CSP buffer";
      }

      int cspPacketHeaderLen = sizeof(packet->padding) + sizeof(packet->length) + sizeof(packet->id);

      // CSP forces us to do our own bookkeeping...
      packet->length = cspPacketDataLengths[currentCSPPacket];
      packet->id.ext = 0x87654321;
      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
        packet->data[i] = (i % 79) + 0x30; // ASCII numbers through to ~
      }

#if QA_CC_HD_DEBUG
      printf("size of packet padding = %ld\n", sizeof(packet->padding));
      printf("size of packet length = %ld\n", sizeof(packet->length));
      printf("size of packet id = %ld\n", sizeof(packet->id));
      printf("size of csp_id_t %ld\n",sizeof(csp_id_t));
      printf("packet length %d (2 bytes) %02x\n", packet->length, packet->length);
      printf("packet id (4 bytes) %04x\n", packet->id);
      printf("Padding\n\t");
      for (uint8_t p = 0; p < sizeof(packet->padding); p++) {
        printf("%02x",packet->padding[p]);
      }
#endif

      // The CSP packet needs to be in a PPDU_u8 and passed to the encoder
      uint8_t * pptr = (uint8_t *) packet;
      std::vector<uint8_t> p(pptr, pptr + cspPacketHeaderLen + packet->length);
      PPDU_u8 inputPayload(p);
      // @TODO maybe make the input to the encoder a std::vector<uint8_t> ???
      PPDU_u8 encodedPayload = ccHDCodec->encode(inputPayload);

      // The codeword (encoded payload) is not systematic, so the message and
      // first k bytes of the codeword should have differences.
      bool same = true;
      for (unsigned long i = 0; i < p.size(); i++) {
        same = same & (p[i] == encodedPayload.getPayload()[i]);
      }
      ASSERT_FALSE(same) << "encoded payload matches payload; not possible if codeword is non-systematic";


#if QA_CC_HD_DEBUG
      printf("p len %ld\n",p.size());
      printf("inputPayload len %ld encodedPayload len %ld\n",inputPayload.payloadLength(),encodedPayload.payloadLength());
#endif

      // Decode the encoded payload
      PPDU_u8::payload_t dPayload;
      const PPDU_u8 ecopyPayload(encodedPayload);
      uint32_t bitErrors = ccHDCodec->decode(encodedPayload.getPayload(), 100.0, dPayload);

#if QA_CC_HD_DEBUG
      printf("csp packet len %ld packet len %ld encoded len %ld decoded len %ld\n",
        cspPacketDataLengths[currentCSPPacket], iPayload.size(), ePayload.size(), dPayload.size());
#endif

      // Convolutional decoding cannot tell how many bit errors there might be
      ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0; Convolutional coding does not ";

      // Check the decoded and original messages match
      std::vector<uint8_t> iPayload = inputPayload.getPayload();
      if (bitErrors == 0) {
        same = true;
        for (unsigned long i = 0; i < iPayload.size(); i++) {
          same = same & (iPayload[i] == dPayload[i]);
        }
        ASSERT_TRUE(same) << "decoded payload does not match input payload";
      }

    } // for various CSP packet lengths

  }
  catch (FECException *e) {
    FAIL() << "Should be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_1_2.";
  }

}

TEST(convolutional_codec_hd, r_1_2_ber_match )
{
  /* ----------------------------------------------------------------------
   * Check the rate 1/2 CCSDS codec against expected BER performance.
   *
   * Refer to chapter 8, Proakis, 4th Ed.
   *
   * Generally, confirm that BER of 0.0001 (1e-4) is met at higher SNRs,
   * and not met at lower. Right around 6dB SNR about 1e-4 should be met.
   *
   * Note: To keep the execution time reasonable and not exceed unit testing
   * time limits, only enough bits are generated to get close to the
   * expected BER. A better test would run 10x or more bits...
   * ----------------------------------------------------------------------
   */

  // Check some SNRs that should easily do better than 1e-4 BER
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    60 /* dB */,
    0.0001,
    false,
    10.0);
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    8 /* dB */,
    0.0001,
    false,
    10.0);
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    7 /* dB */,
    0.0001,
    false,
    10.0);
  // According to Proakis, 4th ed, Figure 8-2-21, right near 6dB SNR a BER of
  // 1e-4 is achieved. Check a little above assuming it will do better than 1e-4
  // and a little below assuming it won't
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    6.3 /* dB */,
    0.0001,
    false,
    10.0);
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    5.7 /* dB */,
    0.0001,
    true,
    10.0);
  // At 5 dB SNR, 1e-4 BER will always be exceeded
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    5 /* dB */,
    0.0001,
    true,
    10.0);
  // At 0 dB SNR, 1e-4 BER will be exceeded really quickly -- see the log.
  check_decoder_ber (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
    5 /* dB */,
    0.0001,
    true,
    10.0);

}


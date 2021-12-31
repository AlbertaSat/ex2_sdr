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

/*!
 * @brief Check FEC decoding for the scheme provided.
 *
 * @details Using randomly generated messages at the specificy SNR, test if the
 * specified bit-error rate can be achieved.
 *
 * @param errorCorrectionScheme The error correction scheme.
 * @param snr The signal to noise ratio of the generated data in the range [-20,60]
 * @param ber The target bit-error rate in the range [1e-6,0.1]
 */
void
check_decoder (
    ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme,
    float snr,
    float ber)
{
  if (ber > 0.1f or ber < 1e-6) {
    printf("The BER must be in the range [1e-6,0.1]\n");
    throw std::exception();
  }

  if (snr > 60.0f or snr < -20.0f) {
    printf("The SNR must be in the range [-20,60]\n");
    throw std::exception();
  }

  ConvolutionalCodecHD ccHDCodec(errorCorrectionScheme);

  ErrorCorrection ec(errorCorrectionScheme, MPDU::maxMTU()*8);


  // Get ready for simulating noise at the input SNR
  float sigma2 = 1.0 / pow (10.0, snr / 10.0); // noise variance
  boost::mt19937 *rng = new boost::mt19937 ();
  rng->seed (time (NULL));
  boost::normal_distribution<> distribution (-sqrt(sigma2), sqrt(sigma2));
  boost::variate_generator<boost::mt19937, boost::normal_distribution<> > dist (*rng, distribution);

  // Deal with data in unpacked format, i.e., 1 bit per byte.
  // The message length is returned in bits. It's easiest to handle the data as
  // being 1 bit per 8-bit symbol, aka unpacked
  unsigned int payloadBitCount = ec.getMessageLen();
  printf("payloadBitCount %ld\n", payloadBitCount);
  PPDU_u8::payload_t unpackedData(payloadBitCount);

  // Calculate the number of bits needed for the desired BER. Assume at least 5
  // errors are needed to establish the BER, so double that to be sure.
  uint32_t numBitsForBER = 2*(5 / ber);
  uint32_t numBits = 0;
  uint32_t numErrors = 0;

  std::srand(std::time(0));
  while (numBits < numBitsForBER)
  {
    // Make a random data payload
    for (unsigned int i = 0; i < payloadBitCount; i++)
      unpackedData[i] = (uint8_t) (std::rand () & 0x0001);
    PPDU_u8 dataPPDU (unpackedData, PPDU_u8::BitsPerSymbol::BPSymb_1);
    dataPPDU.repack(PPDU_u8::BPSymb_8);

    // Encode the packet
    PPDU_u8 encodedPPDU = ccHDCodec.encode (dataPPDU);
    PPDU_u8::payload_t payload = encodedPPDU.getPayload ();

#if QA_CC_HD_DEBUG
    // Make a reference copy
    PPDU_u8::payload_t refPayload(payload);
    for (uint32_t i = 0; i < 30; i++)
      printf("%d ",refPayload[i]);
    printf("\n");
#endif

    VectorTools vc;
    PPDU_f::payload_t payloadFloat;
    vc.bytesToFloat(payload, false, true, true, 1.0f, payloadFloat);

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
    float threshold = 0.5;
    vc.floatToBytes(threshold, false, payloadFloat, payloadPlusNoise);

    // Try to decode the noisy codeword
    PPDU_u8::payload_t decodedPayload;
    numErrors += ccHDCodec.decode(payloadPlusNoise, snr, decodedPayload);

#if QA_CC_HD_DEBUG
    if(!decodedOkay) {
      int32_t diffs = 0;
      for (uint32_t i = 0; i < refPayload.size(); i++) {
        if (decodedPayload[i] != refPayload[i]) {
          printf("diff at index %d\n",i);
          diffs++;
        }
      }
      printf("num diff payload bits = %d\n",diffs);
    }
#endif
    numBits += payloadBitCount;
  } // while not enough bits for BER

  std::string ecn = ec.ErrorCorrectionName(errorCorrectionScheme);
  ASSERT_TRUE(numErrors < 5) << (boost::format ("Decoding failed for %1% @ %2% dB SNR") % ecn % snr).str();
} // check decoder


/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(convolutional_codec_hd, constructor_accessor )
{
  /* ----------------------------------------------------------------------
   * Confirm rate 2/3, 3/4, 5/6, and 7/8 CCSDS cannoy be instantiated.
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

TEST(convolutional_codec_hd, simple_encode_decode_no_errs )
{
  /* ----------------------------------------------------------------------
   * Create rate 1/2 CCSDS codec and test encode and decode with no errors
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

      std::vector<uint8_t> p;

      uint8_t * pptr = (uint8_t *) packet;
      for (int i = 0; i < cspPacketHeaderLen; i++) {
        p.push_back(pptr[i]);
      }
      // This is ugly, so maybe we need to rethink using PPDU_xx?
      for (unsigned long i = 0; i < packet->length; i++) {
        p.push_back(packet->data[i]);
      }

  #if QA_CC_HD_DEBUG
      // Look at the contents :-)
      for (int i = 0; i < p.size(); i++) {
        printf("p[%d] = 0x%02x\n", i, p[i]);
      }
  #endif

      // @TODO maybe make these std::vector<uint8_t> ???
      printf("p len %ld\n",p.size());
      PPDU_u8 inputPayload(p);
      PPDU_u8 encodedPayload = ccHDCodec->encode(inputPayload);
      printf("inputPayload len %ld encodedPayload len %ld\n",inputPayload.payloadLength(),encodedPayload.payloadLength());

      bool same = true;
      std::vector<uint8_t> iPayload = inputPayload.getPayload();
      std::vector<uint8_t> ePayload = encodedPayload.getPayload();
//      for (unsigned long i = 0; i < iPayload.size(); i++) {
//        same = same & (iPayload[i] == ePayload[i]);
//      }
//
//      ASSERT_FALSE(same) << "encoded payload matches input payload; not encoded!";


      PPDU_u8::payload_t dPayload;
      const PPDU_u8 ecopyPayload(encodedPayload);
      uint32_t bitErrors = ccHDCodec->decode(encodedPayload.getPayload(), 100.0, dPayload);

      printf("csp packet len %ld packet len %ld encoded len %ld decoded len %ld\n",
        cspPacketDataLengths[currentCSPPacket], iPayload.size(), ePayload.size(), dPayload.size());

      ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0";

      if (bitErrors == 0) {
        same = true;
        for (unsigned long i = 0; i < iPayload.size(); i++) {
          same = same & (iPayload[i] == dPayload[i]);
        }
        ASSERT_TRUE(same) << "decoded payload does not match input payload";
      }



      // confirm encoded length

      // decode packet

      // compare with original

    } // for various CSP packet lengths

  }
  catch (FECException *e) {
    FAIL() << "Should be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_1_2.";
  }

}

TEST(convolutional_codec_hd, encode_decode_correctable_errs )
{
  check_decoder (ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
      60 /* dB */,
      0.001);

}


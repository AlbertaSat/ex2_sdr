/*!
 * @file qa_lpdc.cpp
 * @author Steven Knudsen
 * @date August 7, 2019
 *
 * @details Unit test for the LDPC encoder and decoder.
 *
 * This unit test exercises the LDPC encoder and decoder for various codeword lengths and rates
 * using randomly generated multiple messages.
 *
 * @copyright Xiphos Systems Corp. 2019
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <random>
#include <time.h>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <eigen3/Eigen/Dense>

#include "error_correction.h"
#include "ldpc.h"
#include "ppdu_f.h"
#include "ppdu_u8.h"
#include "vectorTools.h"
#include "gtest/gtest.h"


using namespace std;
using namespace xiphos::darkstar;

#define QA_LDPC_DEBUG 0
#define QA_IGNORE_LONG_TEST 0 // set to 1 to ignore long duration tests

/*!
 * @brief Check LDPC encoding of random data.
 *
 * @details For each encoding scheme and rate, encode randomly generated data
 * and check that the result is orthogonal (i.e., correct).
 *
 * @param errorCorrectionCoding The error coding scheme.
 * @param codingRate The coding rate
 * @param trials The number of data sets (trials) for the encoding check.
 */
void
check_encoder (
    ErrorCorrection::ErrorCorrectionCoding errorCorrectionCoding,
    ErrorCorrection::CodingRate codingRate,
    unsigned int trials)
{
  ErrorCorrection ec(errorCorrectionCoding, codingRate);
  ParityCheck PC (errorCorrectionCoding, codingRate);

  LDPC Q (false, errorCorrectionCoding, codingRate);

  // Deal with data in unpacked format, i.e., 1 bit per byte.
  unsigned int payloadBitCount = ec.getMessageLen();
  PPDU_u8::payload_t unpackedData(payloadBitCount);

  std::srand(std::time(0));
  for (unsigned int trial = 0; trial < trials; trial++)
  {
    // Make a random data payload
    for (unsigned int i = 0; i < payloadBitCount; i++)
      unpackedData[i] = (uint8_t) (std::rand () & 0x0001);
    PPDU_u8 dataPPDU (unpackedData, PPDU_u8::BitsPerSymbol::BPSymb_1);
    // Encode the packet
    PPDU_u8 encodedPPDU = Q.encodeSparse(dataPPDU);
    PPDU_u8::payload_t payload = encodedPPDU.getPayload ();

    // Check the encoded payload against the parity matrix
    Eigen::VectorXd c (payload.size ());
    for (unsigned int i = 0; i < payload.size (); i++)
      c[i] = payload[i];

    // Checkthe payload using the "dense" parityCheckMatrix
    Eigen::MatrixXd H = PC.parityCheckMatrixDouble();
    Eigen::VectorXd z = H * c;
    // Make binary
    for (unsigned int i = 0; i < z.size (); i++) {
      z[i] = ((uint8_t) z[i]) % 2;
    }

#if QA_LDPC_DEBUG
    printf ("Trial %d : H*c' = %g\n",trial, z.sum ());
#endif
    ASSERT_TRUE(z.sum() == 0) << (boost::format ("Encoded message check H*c' = 0 failed")).str ();

    // DO again for the sparse version of H
    Eigen::SparseMatrix<double>  Hs = PC.parityCheckMatrixSparse();
    Eigen::VectorXd zs;
    zs = Hs * c;
    // Make binary
    for (unsigned int i = 0; i < zs.size (); i++) {
      zs[i] = ((uint8_t) zs[i]) % 2;
    }
    ASSERT_TRUE(zs.sum() == 0) << (boost::format ("Encoded message check H*c' = 0 failed for sparse H")).str ();
  }
}

/*!
 * @brief Check LDPC decoding for the scheme and rate provided.
 *
 * @details Using randomly generated messages at the specificy SNR, test if the
 * specified bit-error rate can be achieved.
 *
 * @param errorCorrectionCoding The error coding scheme.
 * @param codingRate The coding rate
 * @param snr The signal to noise ratio of the generated data in the range [-20,60]
 * @param ber The target bit-error rate in the range [1e-6,0.1]
 */
void
check_decoder (
    ErrorCorrection::ErrorCorrectionCoding errorCorrectionCoding,
    ErrorCorrection::CodingRate codingRate,
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

  ErrorCorrection ec(errorCorrectionCoding, codingRate);
  LDPC Q (false, errorCorrectionCoding, codingRate);

  // Get ready for simulating noise at the input SNR
  float sigma2 = 1.0 / pow (10.0, snr / 10.0); // noise variance
  boost::mt19937 *rng = new boost::mt19937 ();
  rng->seed (time (NULL));
  boost::normal_distribution<> distribution (-sqrt(sigma2), sqrt(sigma2));
  boost::variate_generator<boost::mt19937, boost::normal_distribution<> > dist (*rng, distribution);

  // Deal with data in unpacked format, i.e., 1 bit per byte.
  unsigned int payloadBitCount = ec.getMessageLen();
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

    // Encode the packet
    PPDU_u8 encodedPPDU = Q.encode (dataPPDU);
    PPDU_u8::payload_t payload = encodedPPDU.getPayload ();

#if QA_LDPC_DEBUG
    // We should assume that the encoder unit test works, so the code here
    // is just in case we have trouble with the decoder...
    //
    // Check the encoded payload against the parity matrix
    Eigen::MatrixXi hi = Q.getParityMatrix ();
    Eigen::MatrixXd H = hi.cast<double> ();
    Eigen::VectorXd c (payload.size ());
    for (unsigned int i = 0; i < payload.size (); i++)
      c[i] = payload[i];
    Eigen::VectorXd z = H * c;
    // Make binary
    for (unsigned int i = 0; i < z.size (); i++) {
      z[i] = ((uint8_t) z[i]) % 2;
    }

    printf ("Trial %d : H*c' = %g\n",trial, z.sum ());
    ASSERT_TRUE(z.sum() == 0) << (boost::format ("Encoded message check H*c' = 0 failed")).str ();
#endif

    PPDU_u8::payload_t refPayload(payload);
#if QA_LDPC_DEBUG
    for (uint32_t i = 0; i < 30; i++)
      printf("%d ",refPayload[i]);
    printf("\n");
#endif

    VectorTools vc;
    PPDU_f::payload_t payloadFloat;
    vc.bytesToFloat(payload, false, true, true, 1.0f, payloadFloat);

    // Add noise. The bytesToFloat method makes float symbols of mag 1.
#if QA_LDPC_DEBUG
    float pSignal = 0.0f;
    float pNoise = 0.0f;
#endif
    float noise;
    for (uint32_t i = 0; i < payloadFloat.size(); i++) {
      noise = dist();
#if QA_LDPC_DEBUG
      pNoise += noise*noise;
      pSignal += payloadFloat[i]*payloadFloat[i];
#endif
      payloadFloat[i] += noise;
    }

#if QA_LDPC_DEBUG
    pSignal /= (float) payloadFloat.size();
    pNoise /= (float) payloadFloat.size();
    printf("pSignal = %g pNoise = %g snr = %g\n", pSignal, pNoise, 10.0f*std::log10(pSignal/pNoise));
#endif
    // Try to decode the noisy codeword
    PPDU_u8::payload_t decodedPayload;
    numErrors += Q.decode(payloadFloat, snr, decodedPayload);

#if QA_LDPC_DEBUG
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

  std::string ecn = ec.ErrorCorrectionName(errorCorrectionCoding);
  std::string rc = ec.CodingRateName(codingRate);
  ASSERT_TRUE(numErrors < 5) << (boost::format ("Decoding failed for %1% %2% @ %3% dB SNR") % ecn % rc % snr).str ();
}


/*!
 * @brief
 */
TEST(LDPC, Accessors )
{
  //----------------------------------------------------------------------
  // Test Outline
  //----------------------------------------------------------------------
  // Construct new LDPC object
  //   Check accessors
  //

  try
  {
    LDPC Q1 (false, ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_1_2);

    uint32_t defaultDecodeIterations = Q1.getDecodeIterations();
    ASSERT_TRUE(defaultDecodeIterations == LDPC::DECODE_ITERATIONS_DEFAULT);

    // Check accessors
    defaultDecodeIterations++;
    Q1.setDecodeIterations(defaultDecodeIterations);
    uint32_t currentDecodeIterations = Q1.getDecodeIterations();
    ASSERT_TRUE(defaultDecodeIterations == currentDecodeIterations);
    ASSERT_TRUE(Q1.getCodewordLength() == 648);
    ASSERT_TRUE(Q1.getMessageLength() == 648/2);
  }
  catch (std::runtime_error &re)
  {
    GTEST_FAIL() << boost::format ("LDPC constructor failed.").str() << endl;
  }
}

/*!
 * @brief Test the LDPC encoder.
 *
 * @todo This only checks the IEEE LDPC codes
 */
TEST(LDPC, Encode_IEEE_xxxx )
{
  //----------------------------------------------------------------------
  // Test Outline
  //----------------------------------------------------------------------
  // For each codeword size and rate combination, encode and check codeword
  // is orthogonal to the parity check matrix using 1 or more random payloads
#if !QA_IGNORE_LONG_TEST
  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_1_2, 100);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_2_3, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_3_4, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_5_6, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_1_2, 100);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_2_3, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_3_4, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_5_6, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_1_2, 100);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_2_3, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_3_4, 1);

  check_encoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_5_6, 1);
#endif
}

/*!
 * @brief Test the LDPC encoder.
 *
 * @todo This only checks the IEEE LDPC codes
 */
TEST(LDPC, Decode_IEEE_xxxx )
{
  //----------------------------------------------------------------------
  // Test Outline
  //----------------------------------------------------------------------
  // For each codeword size and rate combination, encode and decode random
  // payloads and test against reasonable BER performance expectations.
  // That is, approach the lower limit SNR for a given codeword and rate.
#if !QA_IGNORE_LONG_TEST
  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_1_2, 7.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_2_3, 8.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_3_4, 9.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
      ErrorCorrection::CodingRate::RATE_5_6, 10.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_1_2, 7.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_2_3, 8.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_3_4, 9.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
      ErrorCorrection::CodingRate::RATE_5_6, 10.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_1_2, 7.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_2_3, 8.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_3_4, 9.0f, 0.001);

  check_decoder (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_5_6, 10.0f, 0.001);
#endif
}

/*!
 * @brief Test the LDPC encoder.
 *
 * @todo This only checks the IEEE LDPC codes
 */
TEST(LDPC, MatrixMultSpeed )
{

  ErrorCorrection ec(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_1_2);
  ParityCheck PC (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_1_2);

  LDPC Q (false, ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
      ErrorCorrection::CodingRate::RATE_1_2);

  // Deal with data in unpacked format, i.e., 1 bit per byte.
  unsigned int payloadBitCount = ec.getMessageLen();
  PPDU_u8::payload_t unpackedData(payloadBitCount);

  // Make a random data payload
  std::srand(std::time(0));
  for (unsigned int i = 0; i < payloadBitCount; i++)
    unpackedData[i] = (uint8_t) (std::rand () & 0x0001);
  PPDU_u8 dataPPDU (unpackedData, PPDU_u8::BitsPerSymbol::BPSymb_1);
  // Encode the packet
  PPDU_u8 encodedPPDU = Q.encodeSparse(dataPPDU);
  PPDU_u8::payload_t payload = encodedPPDU.getPayload ();

  // Check the encoded payload against the parity matrix
  Eigen::VectorXd c (payload.size ());
  for (unsigned int i = 0; i < payload.size (); i++)
    c[i] = payload[i];

  Eigen::VectorXd z;
  Eigen::MatrixXd H = PC.parityCheckMatrixDouble();
  auto t0 = std::chrono::high_resolution_clock::now();
  for (unsigned int trial = 0; trial < 1000; trial++)
  {
    z = H * c;
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  auto durationDense = std::chrono::duration_cast<std::chrono::microseconds>( t1 - t0 ).count();

  // DO again for the sparse version of H
  Eigen::SparseMatrix<double>  Hs = PC.parityCheckMatrixSparse();
  t0 = std::chrono::high_resolution_clock::now();
  for (unsigned int trial = 0; trial < 1000; trial++)
  {
    z = Hs * c;
  }
  t1 = std::chrono::high_resolution_clock::now();
  auto durationSparse = std::chrono::duration_cast<std::chrono::microseconds>( t1 - t0 ).count();

#if QA_LDPC_DEBUG
  std::cout << "1000 dense matrix multiplcation takes " << durationDense << " microseconds" << std::endl;
  std::cout << "1000 sparse matrix multiplcation takes " << durationSparse << " microseconds" << std::endl;
#endif

  ASSERT_TRUE(durationDense > durationSparse) << (boost::format ("Somehow sparse matrix multiplication is slow!?!")).str ();
}



/*!
 * @file qa_mpdu.cpp
 * @author Steven Knudsen
 * @date June 6, 2021
 *
 * @details Unit test for the MPDU class.
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

//#include "error_correction.hpp"
#include "mpdu.hpp"
#include "mpduHeader.hpp"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MPDU_DEBUG 0 // set to 1 for debugging output

#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // UHF transparent mode packet is always 128 bytes



/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
//TEST(mpduHeader, ConstructorParemeterized )
//{
//  /* ---------------------------------------------------------------------
//   * Confirm the MAC PDU is constructed and correct
//   * ---------------------------------------------------------------------
//   */
//
//  // We will take a look at the raw header bits to confirm things are correct.
//  //
//
//  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_0; // 0b000
//  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
//      ErrorCorrection::ErrorCorrectionScheme::CONVOLUTIONAL_CODING_R_1_2; // 0b000000
//  uint8_t codewordFragmentIndex = 0;
//  uint16_t userPacketLength = 0;
//  uint8_t userPacketFragmentIndex = 0;
//
//  MPDUHeader *header1, *header2;
//
//  for (uint16_t m = (uint16_t)RF_Mode::RF_ModeNumber::RF_MODE_0; m <= (uint16_t) RF_Mode::RF_ModeNumber::RF_MODE_7; m++) {
//    modulation = static_cast<RF_Mode::RF_ModeNumber>(m);
//
//    for (uint16_t e = (uint16_t) ErrorCorrection::ErrorCorrectionScheme::CONVOLUTIONAL_CODING_R_1_2;
//        e < (uint16_t) ErrorCorrection::ErrorCorrectionScheme::LAST; e++) {
//      errorCorrectionScheme = static_cast<ErrorCorrection::ErrorCorrectionScheme>(e);
//
//      for (codewordFragmentIndex = 0; codewordFragmentIndex < 0x80; codewordFragmentIndex++) {
//
//        // Really can't iterate over all packet lengths and fragments, so just do some
//        for (userPacketLength = 0; userPacketLength < 0x0100; userPacketLength++ ) {
//          for (userPacketFragmentIndex = 0; userPacketFragmentIndex < 0x04; userPacketFragmentIndex++ ) {
//
//            header1 = new MPDUHeader(UHF_TRANSPARENT_MODE_PACKET_LENGTH,
//              modulation,
//              errorCorrectionScheme,
//              codewordFragmentIndex,
//              userPacketLength,
//              userPacketFragmentIndex);
//
//            std::vector<uint8_t> payload1 = header1->getHeaderPayload();
//
//            // Make the payload long enough
//            payload1.resize(UHF_TRANSPARENT_MODE_PACKET_LENGTH + 1);
//            header2 = new MPDUHeader(payload1);
//
//            // Check headers match
//            ASSERT_TRUE(headersSame(header1, header2)) << "Oops, header packets don't match!";
//
//          } // over all user packet fragment indices
//        } // over all user packet lengths
//
//      } // over all codeword fragment indices
//    } // over all error correction schemes
//  } // over all modulations
//}

/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(mpdu, Constructor)
{
  /* ---------------------------------------------------------------------
   * Check all accessors for both constructors
   * ---------------------------------------------------------------------
   */

  std::vector<uint8_t> rawHeader(9,0);


  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2; // 0b000000
  uint8_t codewordFragmentIndex = 0x55;
  uint16_t userPacketLength = 1234; // 0x04d2
  uint8_t userPacketFragmentIndex = 0xAA;

  MPDUHeader *header1;

  header1 = new MPDUHeader(UHF_TRANSPARENT_MODE_PACKET_LENGTH,
    modulation,
    errorCorrectionScheme,
    codewordFragmentIndex,
    userPacketLength,
    userPacketFragmentIndex);

  uint16_t len = header1->MACHeaderLength()/8; // bytes

  std::vector<uint8_t> codeword(UHF_TRANSPARENT_MODE_PACKET_LENGTH - len, 0xAA);
  MPDU *mpdu1;
  mpdu1 = new MPDU(*header1, codeword);

  std::vector<uint8_t> rawMPDU = mpdu1->getMPDU();
  ASSERT_TRUE(rawMPDU.size() == (uint32_t) MPDU_LENGTH) << "MPDU length incorrect!";

#if QA_MPDU_DEBUG
  for (uint16_t i = 0; i < rawMPDU.size(); i++) {
    printf("rawPDU[%d] = 0x%02x\n", i, rawMPDU[i]);
  }
#endif

  MPDU *mpdu2 = new MPDU(rawMPDU);
  MPDUHeader *header2 = mpdu2->getMpduHeader();

  RF_Mode::RF_ModeNumber modulationAccess = header2->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ErrorCorrection::ErrorCorrectionScheme ecScheme = header2->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  uint8_t cwFragmentIndex = header2->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uint16_t uPacketLen = header2->getUserPacketLength();
  ASSERT_TRUE(userPacketLength == uPacketLen) << "User packet lenghts don't match!";

  uint8_t uPacketFragIndex = header2->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  uint16_t headerLength = header2->MACHeaderLength();
  ASSERT_TRUE(headerLength == header1->MACHeaderLength()) << "Header length is wrong!";

}


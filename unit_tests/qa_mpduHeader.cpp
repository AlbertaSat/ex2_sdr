/*!
 * @file qa_golay.cpp
 * @author Steven Knudsen
 * @date May 28, 2021
 *
 * @details Unit test for the golay codec.
 *
 * This unit test exercises the golay codec.
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

#include "error_correction.hpp"
#include "mpduHeader.hpp"
#include "rfMode.hpp"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MPDUHEADER_DEBUG 0 // set to 1 for debugging output


#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // UHF transparent mode packet is always 128 bytes
/*!
 * @brief Factorial n!
 */
uint64_t factorial(uint16_t n) {
  uint64_t f = 1;
  for (uint16_t i = 0; i < n; i++) {
    f *= i;
  }
  return f;
}

uint64_t combination (uint16_t k, uint16_t n) {
  uint64_t c = factorial(n) * factorial (k - n);
  c = factorial(k) / c;
  return c;
}

/*!
 * @brief Generate an error pattern of n bits for a 24 bit word.
 *
 *
 * @param n
 * @return
 */

uint32_t nBitErrorPattern(uint8_t n) {
  uint32_t pattern = 0;

  vector<uint8_t> positions(24,0);
  uint8_t count = 0;
  while (count < n) {
    uint8_t bitPosition = random() % (24);
    if (positions[bitPosition] == 0) {
      positions[bitPosition] = 1;
      uint32_t bitPattern = (0x00000001 << bitPosition) & 0x00ffffff;
      pattern = pattern | bitPattern;
      count++;
    }
  }
  return pattern;
}

bool headersSame(MPDUHeader *h1, MPDUHeader *h2) {
  bool same = true;

#if QA_MPDUHEADER_DEBUG
  printf(" 0x%04x 0x%04x\n",h1->getUhfPacketLength(), h2->getUhfPacketLength());
  printf(" 0x%04x 0x%04x\n",h1->getRfModeNumber(), h2->getRfModeNumber());
  printf(" 0x%04x 0x%04x\n",h1->getErrorCorrectionScheme(), h2->getErrorCorrectionScheme());
  printf(" 0x%04x 0x%04x\n",h1->getCodewordFragmentIndex(), h2->getCodewordFragmentIndex());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketLength(), h2->getUserPacketLength());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketFragmentIndex(), h2->getUserPacketFragmentIndex());
#endif

  same &= (h1->getUhfPacketLength() == h2->getUhfPacketLength());
  same &= (h1->getRfModeNumber() == h2->getRfModeNumber());
  same &= (h1->getErrorCorrectionScheme() == h2->getErrorCorrectionScheme());
  same &= (h1->getCodewordFragmentIndex() == h2->getCodewordFragmentIndex());
  same &= (h1->getUserPacketLength() == h2->getUserPacketLength());
  same &= (h1->getUserPacketFragmentIndex() == h2->getUserPacketFragmentIndex());

  return same;
}
/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(mpduHeader, ConstructorParemeterized )
{
  /* ---------------------------------------------------------------------
   * Confirm the MAC header is constructed and represents a good header
   * ---------------------------------------------------------------------
   */

  // We will take a look at the raw header bytes to confirm things are correct.
  //

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_0; // 0b000
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::CONVOLUTIONAL_CODING_R_1_2; // 0b000000
  uint8_t codewordFragmentIndex = 0;
  uint16_t userPacketLength = 0;
  uint8_t userPacketFragmentIndex = 0;

  MPDUHeader *header1, *header2;

  for (uint16_t m = (uint16_t)RF_Mode::RF_ModeNumber::RF_MODE_0; m <= (uint16_t) RF_Mode::RF_ModeNumber::RF_MODE_7; m++) {
    modulation = static_cast<RF_Mode::RF_ModeNumber>(m);

    for (uint16_t e = (uint16_t) ErrorCorrection::ErrorCorrectionScheme::CONVOLUTIONAL_CODING_R_1_2;
        e < (uint16_t) ErrorCorrection::ErrorCorrectionScheme::LAST; e++) {
      errorCorrectionScheme = static_cast<ErrorCorrection::ErrorCorrectionScheme>(e);

      for (codewordFragmentIndex = 0; codewordFragmentIndex < 0x80; codewordFragmentIndex++) {

        // Really can't iterate over all packet lengths and fragments, so just do some
        for (userPacketLength = 0; userPacketLength < 0x0100; userPacketLength++ ) {
          for (userPacketFragmentIndex = 0; userPacketFragmentIndex < 0x04; userPacketFragmentIndex++ ) {

            header1 = new MPDUHeader(UHF_TRANSPARENT_MODE_PACKET_LENGTH,
              modulation,
              errorCorrectionScheme,
              codewordFragmentIndex,
              userPacketLength,
              userPacketFragmentIndex);

            std::vector<uint8_t> payload1 = header1->getHeaderPayload();

            // Make the payload long enough
            payload1.resize(UHF_TRANSPARENT_MODE_PACKET_LENGTH + 1);
            header2 = new MPDUHeader(payload1);

            // Check headers match
            ASSERT_TRUE(headersSame(header1, header2)) << "Oops, header packets don't match!";

          } // over all user packet fragment indices
        } // over all user packet lengths

      } // over all codeword fragment indices
    } // over all error correction schemes
  } // over all modulations
}

/*!
 * @brief Test FEC capabilities
 *
 * @note: Since the golay unit is fully tested, this test of the ability of
 * the MPDUHeader that takes the received packet as input only tests its
 * ability to bork when it there are 4 bit errors in a Golay codeword (any one
 * of 3 in the header).
 */
//TEST(golay, ConstructorRecdPacket )
//{
//}

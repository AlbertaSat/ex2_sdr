/*!
 * @file qa_mpduHeader.cpp
 * @author Steven Knudsen
 * @date June 4, 2021
 *
 * @details Unit test for the MPDU Header class.
 *
 * @note The Constructor test attempts to do many, but not all parameter combinations. Meson
 * times out even when I set a long timeout in the build file, so not sure what to do.
 * Not super important as there is enough tromping throught the parameter combinations
 * to be pretty confident this works.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "error_correction.hpp"
#include "mpdu.hpp"
#include "radio.h"
#include "rfMode.hpp"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MPDUHEADER_DEBUG 0 // set to 1 for debugging output

bool headersSame(MPDUHeader *h1, MPDUHeader *h2) {
  bool same = true;

#if QA_MPDUHEADER_DEBUG
  /*printf(" 0x%04x 0x%04x\n",h1->getUhfPacketLength(), h2->getUhfPacketLength());*/
  printf(" 0x%04x 0x%04x\n",h1->getRfModeNumber(), h2->getRfModeNumber());
  printf(" 0x%04x 0x%04x\n",h1->getErrorCorrectionScheme(), h2->getErrorCorrectionScheme());
  printf(" 0x%04x 0x%04x\n",h1->getCodewordFragmentIndex(), h2->getCodewordFragmentIndex());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketLength(), h2->getUserPacketLength());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketFragmentIndex(), h2->getUserPacketFragmentIndex());
#endif

  /*same &= (h1->getUhfPacketLength() == h2->getUhfPacketLength());*/
  same &= (h1->getRfModeNumber() == h2->getRfModeNumber());
  same &= (h1->getErrorCorrectionScheme() == h2->getErrorCorrectionScheme());
  same &= (h1->getCodewordFragmentIndex() == h2->getCodewordFragmentIndex());
  same &= (h1->getUserPacketLength() == h2->getUserPacketLength());
  same &= (h1->getUserPacketFragmentIndex() == h2->getUserPacketFragmentIndex());

  return same;
}

/*!
 * @brief Test constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(mpduHeader, ConstructorParemeterized )
{
  /* ---------------------------------------------------------------------
   * Confirm the MAC header is constructed and represents a good header
   * ---------------------------------------------------------------------
   */

  // We will take a look at the raw header bits to confirm things are correct.
  //

  RF_Mode::RF_ModeNumber modulation;
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme;
  uint8_t codewordFragmentIndex = 0;
  uint16_t userPacketLength = 0;
  uint8_t userPacketFragmentIndex = 0;

  MPDUHeader *header1, *header2;

  for (uint16_t m = (uint16_t)RF_Mode::RF_ModeNumber::RF_MODE_0; m <= (uint16_t) RF_Mode::RF_ModeNumber::RF_MODE_7; m++) {
    modulation = static_cast<RF_Mode::RF_ModeNumber>(m);

    for (uint16_t e = (uint16_t) ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        e < (uint16_t) ErrorCorrection::ErrorCorrectionScheme::LAST; e++) {

//      printf("scheme = %d\n",e);

      errorCorrectionScheme = static_cast<ErrorCorrection::ErrorCorrectionScheme>(e);
      if (ErrorCorrection::isValid(errorCorrectionScheme)) {

        ErrorCorrection errorCorrection(errorCorrectionScheme, (MPDU::maxMTU() * 8));

        for (codewordFragmentIndex = 0; codewordFragmentIndex < 0x80; codewordFragmentIndex++) {

          // Really can't iterate over all packet lengths and fragments, so just do some
          for (userPacketLength = 0; userPacketLength < 0x0100; userPacketLength++ ) {
            for (userPacketFragmentIndex = 0; userPacketFragmentIndex < 0x04; userPacketFragmentIndex++ ) {
              //            printf("pre header \n");

              header1 = new MPDUHeader(/*UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH,*/
                modulation,
                errorCorrection,
                codewordFragmentIndex,
                userPacketLength,
                userPacketFragmentIndex);

              ASSERT_TRUE(header1 != NULL) << "MPDUHeader 1 failed to instantiate";

              std::vector<uint8_t> payload1 = header1->getHeaderPayload();

//              printf("payload1 size %ld\n", payload1.size());

              // Make the payload long enough
//              payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH);
              try {
                header2 = new MPDUHeader(payload1);
              }
              catch (MPDUHeaderException &e) {
                FAIL() << e.what();
              }
              ASSERT_TRUE(header2 != NULL) << "MPDUHeader 2 failed to instantiate";

              // Check headers match
              ASSERT_TRUE(headersSame(header1, header2)) << "Oops, header packets don't match!";

            } // over all user packet fragment indices
          } // over all user packet lengths

        } // over all codeword fragment indices

      } // valid error correction scheme?

    } // over all error correction schemes
  } // over all modulations
}

/*!
 * @brief Test all accessors
 */
TEST(mpduHeader, Accessors )
{
  /* ---------------------------------------------------------------------
   * Check all accessors for both constructors
   * ---------------------------------------------------------------------
   */

// @todo do this for all supported FEC schemes, maybe in combination with RF modes?

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2; // 0b000000
  uint8_t codewordFragmentIndex = 0x55;
  uint16_t userPacketLength = 1234; // 0x04d2
  uint8_t userPacketFragmentIndex = 0xAA;

  MPDUHeader *header1, *header2;


  header1 = new MPDUHeader(/*UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH,*/
    modulation,
    errorCorrectionScheme,
    codewordFragmentIndex,
    userPacketLength,
    userPacketFragmentIndex);

  RF_Mode::RF_ModeNumber modulationAccess = header1->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ErrorCorrection::ErrorCorrectionScheme ecScheme = header1->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  uint8_t cwFragmentIndex = header1->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uint16_t uPacketLen = header1->getUserPacketLength();
  ASSERT_TRUE(userPacketLength == uPacketLen) << "User packet lenghts don't match!";

  uint8_t uPacketFragIndex = header1->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  uint16_t headerLength = header1->MACHeaderLength();
  ASSERT_TRUE(headerLength == MPDUHeader::MACHeaderLength()) << "Header length is wrong!";


  std::vector<uint8_t> payload1 = header1->getHeaderPayload();

  // Make the payload long enough
  payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + 1);
  header2 = new MPDUHeader(payload1);

  modulationAccess = header2->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ecScheme = header2->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  cwFragmentIndex = header2->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uPacketLen = header2->getUserPacketLength();
  ASSERT_TRUE(userPacketLength == uPacketLen) << "User packet lenghts don't match!";

  uPacketFragIndex = header2->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  headerLength = header2->MACHeaderLength();
  ASSERT_TRUE(headerLength == MPDUHeader::MACHeaderLength()) << "Header length is wrong!";

}


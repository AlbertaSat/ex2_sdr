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

#define QA_MPDUHEADER_DEBUG 0         // set to 1 for debugging output
#define QA_MPDUHEADER_DEBUG_VERBOSE 0 // set to 1 for verbose debugging output

bool headersSame(MPDUHeader *h1, MPDUHeader *h2) {
  bool same = true;

#if QA_MPDUHEADER_DEBUG_VERBOSE
  printf(" 0x%04x 0x%04x\n",(unsigned int) h1->getRfModeNumber(), (unsigned int) h2->getRfModeNumber());
  printf(" 0x%04x 0x%04x\n",(unsigned int) h1->getErrorCorrectionScheme(), (unsigned int) h2->getErrorCorrectionScheme());
  printf(" 0x%04x 0x%04x\n",h1->getCodewordFragmentIndex(), h2->getCodewordFragmentIndex());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketPayloadLength(), h2->getUserPacketPayloadLength());
  printf(" 0x%04x 0x%04x\n",h1->getUserPacketFragmentIndex(), h2->getUserPacketFragmentIndex());
#endif

  /*same &= (h1->getUhfPacketLength() == h2->getUhfPacketLength());*/
  same &= (h1->getRfModeNumber() == h2->getRfModeNumber());
  same &= (h1->getErrorCorrectionScheme() == h2->getErrorCorrectionScheme());
  same &= (h1->getCodewordFragmentIndex() == h2->getCodewordFragmentIndex());
  same &= (h1->getUserPacketPayloadLength() == h2->getUserPacketPayloadLength());
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
  uint16_t userPacketPayloadLength = 0;
  uint8_t userPacketFragmentIndex = 0;

  MPDUHeader *header1, *header2;

  for (uint16_t m = (uint16_t)RF_Mode::RF_ModeNumber::RF_MODE_0; m <= (uint16_t) RF_Mode::RF_ModeNumber::RF_MODE_7; m++) {
    modulation = static_cast<RF_Mode::RF_ModeNumber>(m);

    for (uint16_t e = (uint16_t) ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        e < (uint16_t) ErrorCorrection::ErrorCorrectionScheme::LAST; e++) {

      errorCorrectionScheme = static_cast<ErrorCorrection::ErrorCorrectionScheme>(e);
      if (ErrorCorrection::isValid(errorCorrectionScheme)) {

        ErrorCorrection errorCorrection(errorCorrectionScheme, (MPDU::maxMTU() * 8));

        for (codewordFragmentIndex = 0; codewordFragmentIndex < 0x80; codewordFragmentIndex++) {

          // Really can't iterate over all packet lengths and fragments, so just do some
          for (userPacketPayloadLength = 0; userPacketPayloadLength < 0x0100; userPacketPayloadLength++ ) {
            for (userPacketFragmentIndex = 0; userPacketFragmentIndex < 0x04; userPacketFragmentIndex++ ) {

              header1 = new MPDUHeader(/*UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH,*/
                modulation,
                errorCorrection,
                codewordFragmentIndex,
                userPacketPayloadLength,
                userPacketFragmentIndex);

              ASSERT_TRUE(header1 != NULL) << "MPDUHeader 1 failed to instantiate";

              std::vector<uint8_t> payload1 = header1->getHeaderPayload();

              try {
                header2 = new MPDUHeader(errorCorrection, payload1);
              }
              catch (MPDUHeaderException &e) {
                FAIL() << e.what();
              }
              ASSERT_TRUE(header2 != NULL) << "MPDUHeader 2 failed to instantiate";

              // Check headers match
              ASSERT_TRUE(headersSame(header1, header2)) << "Oops, header packets don't match!";

              delete(header2);
              delete(header1);
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
  ErrorCorrection ec(errorCorrectionScheme, MPDU::maxMTU() * 8);
  uint8_t codewordFragmentIndex = 0x55;
  uint16_t userPacketPayloadLength = 1234; // 0x04d2
  uint8_t userPacketFragmentIndex = 0xAA;

  MPDUHeader *header1, *header2;


  header1 = new MPDUHeader(
    modulation,
    ec,
    codewordFragmentIndex,
    userPacketPayloadLength,
    userPacketFragmentIndex);

  RF_Mode::RF_ModeNumber modulationAccess = header1->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ErrorCorrection::ErrorCorrectionScheme ecScheme = header1->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  uint8_t cwFragmentIndex = header1->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uint16_t uPacketLen = header1->getUserPacketPayloadLength();
  ASSERT_TRUE(userPacketPayloadLength == uPacketLen) << "User packet lenghts don't match!";

  uint8_t uPacketFragIndex = header1->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  uint16_t headerLength = header1->MACHeaderLength();
  ASSERT_TRUE(headerLength == MPDUHeader::MACHeaderLength()) << "Header length is wrong!";

  // Get the raw header payload
  std::vector<uint8_t> payload1 = header1->getHeaderPayload();

  // Turned it into a full packet payload since that is what we expect will be
  // passed to the raw header constructor
  payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + 1);
  try {
    header2 = new MPDUHeader(ec, payload1);

    modulationAccess = header2->getRfModeNumber();
    ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

    ecScheme = header2->getErrorCorrectionScheme();
    ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

    cwFragmentIndex = header2->getCodewordFragmentIndex();
    ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

    uPacketLen = header2->getUserPacketPayloadLength();
    ASSERT_TRUE(userPacketPayloadLength == uPacketLen) << "User packet lenghts don't match!";

    uPacketFragIndex = header2->getUserPacketFragmentIndex();
    ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

    headerLength = header2->MACHeaderLength();
    ASSERT_TRUE(headerLength == MPDUHeader::MACHeaderLength()) << "Header length is wrong!";

  }
  catch(MPDUHeaderException &e) {

  };
  if (!header2)
    delete(header2);
  delete(header1);
}

/*!
 * @brief Test raw header reconstruction including with bad data (bit errors
 * were introduced)
 */
TEST(mpduHeader, RawReconstruction )
{
  /* ---------------------------------------------------------------------
   * Inject header errors and check expected results. In general, Golay
   * (24,12,8) can correct up to 3 bit errors, and detect 4. Any more and
   * the results are inconsistent, so essentially false positives.
   * ---------------------------------------------------------------------
   */

// @todo do this for all supported FEC schemes, maybe in combination with RF modes?
// @todo should really break into multiple unit tests...

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2; // 0b000000
  ErrorCorrection ec(errorCorrectionScheme, MPDU::maxMTU() * 8);
  uint8_t codewordFragmentIndex = 0x55;
  uint16_t userPacketPayloadLength = 1234; // 0x04d2
  uint8_t userPacketFragmentIndex = 0xAA;

  MPDUHeader *header1, *header2;

  // header1 is the reference, good header.
  header1 = new MPDUHeader(
    modulation,
    ec,
    codewordFragmentIndex,
    userPacketPayloadLength,
    userPacketFragmentIndex);

  // Being really redundant, check all the accessors for header1 return the
  // right values.
  RF_Mode::RF_ModeNumber modulationAccess = header1->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ErrorCorrection::ErrorCorrectionScheme ecScheme = header1->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  uint8_t cwFragmentIndex = header1->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uint16_t uPacketLen = header1->getUserPacketPayloadLength();
  ASSERT_TRUE(userPacketPayloadLength == uPacketLen) << "User packet lenghts don't match!";

  uint8_t uPacketFragIndex = header1->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  uint16_t headerLength = header1->MACHeaderLength();
  ASSERT_TRUE(headerLength == MPDUHeader::MACHeaderLength()) << "Header length is wrong!";

  //
  // First test, very bad header
  //

  // Get the raw header payload
  std::vector<uint8_t> payload1 = header1->getHeaderPayload();

  // Turned it into a full packet payload since that is what we expect will be
  // passed to the raw header constructor
  payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + 1);

  // Now, let's mangle the whole raw header
  for (unsigned int i = 0; i < MPDUHeader::MACHeaderLength(); i++) {
    payload1[i] = payload1[i] ^ 0x55;
  }

  // @Note we could use the Google Test "EXPECT_THROW", but I don't seem able
  // to embedded blocks properly, so we do it the old way.

  // We expect problems...
  try {
    header2 = new MPDUHeader(ec, payload1);
    EXPECT_TRUE(false) << "The corrupted raw header should have caused an exception";
  }
  catch(MPDUHeaderException &e) {
#if QA_MPDUHEADER_DEBUG
    printf("With the entire MPDUHeader mangled, exception is ... %s\n",e.what());
#endif
  };
  if (!header2)
    delete(header2);

  //
  // Second test, only the error correction scheme enum is twiddled, but not
  // enough to prevent the packet from working
  //

  // Get the raw header payload
  payload1 = header1->getHeaderPayload();

  // Turned it into a full packet payload since that is what we expect will be
  // passed to the raw header constructor
  payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + 1);

  // Now, let's mangle the MSB of the MPDUHeader error correction enum. The MSB
  // is actually bit 0 of byte 2 of the payload (the first 12 bits are Golay parity).
  uint8_t temp = payload1[1] ^ 0x01; // flip bit 6 of the 6-bit error correction scheme
#if QA_MPDUHEADER_DEBUG
  printf("payload1[1] 0x%02X temp 0x%02X\n",payload1[1],temp);
#endif
  payload1[1] = temp;

  // We expect no problems...
  try {
    header2 = new MPDUHeader(ec, payload1);
  }
  catch(MPDUHeaderException &e) {
    EXPECT_TRUE(false) << "The corrupted raw header should not have caused an exception";
#if QA_MPDUHEADER_DEBUG
    printf("With the 1 bit of MPDUHeader mangled, exception is ... %s\n",e.what());
#endif
  };
  if (!header2)
    delete(header2);

  //
  // third test, 5 of 6 bits in the error correction scheme enum are flipped
  //

  // Get the raw header payload
  payload1 = header1->getHeaderPayload();

  // Turned it into a full packet payload since that is what we expect will be
  // passed to the raw header constructor
  payload1.resize(UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH + 1);

  // Now, let's mangle the 5 LSBs of the 6-bit MPDUHeader error correction enum.
  // The MSB is the last bit in the 2nd byte, the top MSBs of the 3rd byte
  // correspond to the 5 LSBs of the error correction scheme.
  temp = payload1[2] ^ 0xF8;
#if QA_MPDUHEADER_DEBUG
  printf("payload1[2] 0x%02X temp  0x%02X\n",payload1[2], temp);
#endif
  payload1[2] = temp;

  // We expect problems...
  try {
    header2 = new MPDUHeader(ec, payload1);
    EXPECT_TRUE(false) << "The corrupted raw header should not have caused an exception";
  }
  catch(MPDUHeaderException &e) {
#if QA_MPDUHEADER_DEBUG
    printf("With the 5 of 6 error correction enum bits mangled, exception is ... %s\n",e.what());
#endif
  };
  if (!header2)
    delete(header2);

  delete(header1);
}

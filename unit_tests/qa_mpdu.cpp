/*!
 * @file qa_mpdu.cpp
 * @author Steven Knudsen
 * @date June 6, 2021
 *
 * @details Unit test for the MPDU class.
 *
 * @todo More tests required; test exception generation, ...?
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

#include "mpdu.hpp"

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

#define QA_MPDU_DEBUG 0 // set to 1 for debugging output

#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // UHF transparent mode packet is always 128 bytes

uint32_t mpdusPerCSPPacket(csp_packet_t * cspPacket, ErrorCorrection &errorCorrection) {
  // Get length of CSP packet in bytes. Make sure we are not fooled by
  // alignment, so add up the struct members
  uint32_t cspPacketSize = sizeof(csp_packet_t) + cspPacket->length;

  return MPDU::mpdusInNBytes(cspPacketSize, errorCorrection);

} // mpdusPerCSPPacket

uint16_t mpdusPerCodeword(ErrorCorrection &errorCorrection) {

  // Get the FEC scheme codeword lengths in bytes
  uint32_t cwLen = errorCorrection.getCodewordLen() / 8;
  if (errorCorrection.getCodewordLen() % 8 != 0) {
    cwLen++;
  }

  uint32_t numMPDUPayloadPerCW = cwLen / MPDU::maxMTU();
  if (cwLen % MPDU::maxMTU() != 0) {
    numMPDUPayloadPerCW++;
  }

  return numMPDUPayloadPerCW;

} // mpdusPerCodeword

/*!
 * @brief Test Constructors, the one that is parameterized, and the one
 * that takes the received packet as input. Deep compare the two objects to
 * make sure the second constructor is correct.
 */
TEST(mpdu, ConstructorsAndAccessors)
{
  /* ---------------------------------------------------------------------
   * Use the parameterized constructor to check the raw data constructor.
   * The deep comparising exercises all accessors
   * ---------------------------------------------------------------------
   *
   * The raw MPDU can be obtained from an object made with the parameterized
   * constructor. In turn, it can be used as input to the raw constructor.
   *
   * Check that the two objects match by doing a deep comparison. They must match.
   *
   * Note: the actual values for the members are not verified; that is done
   * in the MPPDHeader unit tests.
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2; // 0b000000
  ErrorCorrection errorCorrection(errorCorrectionScheme, (MPDU::maxMTU() * 8));

  // None of these values really make sense, but they don't have to for this test.
  uint8_t codewordFragmentIndex = 0x55;
  uint16_t userPacketLength = 1234; // 0x04d2
  uint8_t userPacketFragmentIndex = 0xAA;

  MPDUHeader *header1;

  header1 = new MPDUHeader(
    modulation,
    errorCorrection,
    codewordFragmentIndex,
    userPacketLength,
    userPacketFragmentIndex);

  ASSERT_TRUE(header1 != NULL) << "MPDUHeader failed to instantiate";

  // What the heck, let's make the data random even though we don't touch it
  // for this unit test
  std::vector<uint8_t> codeword1(MPDU::maxMTU(), 0xAA);
  for (uint16_t i = 0; i < MPDU::maxMTU(); i++) {
    codeword1[i] = rand() % 0xFF;
  }
  // Instantiate an object using the parameterized constructor
  MPDU *mpdu1;
  mpdu1 = new MPDU(*header1, codeword1);
  ASSERT_TRUE(mpdu1 != NULL) << "MPDU 1 failed to instantiate";

  // Do a simple check
  std::vector<uint8_t> rawMPDU = mpdu1->getRawMPDU();
  ASSERT_TRUE(rawMPDU.size() == (uint32_t) UHF_TRANSPARENT_MODE_PACKET_LENGTH) << "MPDU length incorrect!";

#if QA_MPDU_DEBUG
    for (uint16_t i = 0; i < rawMPDU.size(); i++) {
      printf("rawPDU[%d] = 0x%02x\n", i, rawMPDU[i]);
    }
#endif

  // Instantiate a second object using the raw data constructor
  MPDU *mpdu2 = new MPDU(rawMPDU);

  ASSERT_TRUE(mpdu2 != NULL) << "MPDU 2 failed to instantiate";

  // Get the header from the second object
  MPDUHeader *header2 = mpdu2->getMpduHeader();

  // Now check that both headers match
  RF_Mode::RF_ModeNumber modulationAccess = header2->getRfModeNumber();
  ASSERT_TRUE(modulationAccess == modulation) << "modulation aka RF_Mode doesn't match!";

  ErrorCorrection::ErrorCorrectionScheme ecScheme = header2->getErrorCorrectionScheme();
  ASSERT_TRUE(errorCorrectionScheme == ecScheme) << "ErrorCorrectionScheme doesn't match!";

  uint8_t cwFragmentIndex = header2->getCodewordFragmentIndex();
  ASSERT_TRUE(codewordFragmentIndex == cwFragmentIndex) << "codeword fragment indices don't match!";

  uint16_t uPacketLen = header2->getUserPacketPayloadLength();
  ASSERT_TRUE(userPacketLength == uPacketLen) << "User packet lenghts don't match!";

  uint8_t uPacketFragIndex = header2->getUserPacketFragmentIndex();
  ASSERT_TRUE(userPacketFragmentIndex == uPacketFragIndex) << "user packet fragment indices don't match!";

  uint16_t headerLength = header2->MACHeaderLength();
  ASSERT_TRUE(headerLength == header1->MACHeaderLength()) << "Header length is wrong!";

  // Last, check the codewords match
  std::vector<uint8_t> codeword2 = mpdu2->getCodeword();

  if (codeword1.size() == codeword2.size()) {
    bool cwMatch = true;
    for (uint16_t i = 0; i < codeword1.size(); i++) {
      cwMatch = cwMatch && (codeword1[i] == codeword2[i]);
    }
    ASSERT_TRUE(cwMatch) << "Codewords don't match!";
  }
  else {
    ASSERT_TRUE(false) << "Codeword lengths don't match!";

  }
}

/*!
 * @brief Test the accessors using the two Constructors
 */
TEST(mpdu, NonAccessorMethods)
{
  /* ---------------------------------------------------------------------
   * Check the non-accessor methods for objects made with both constructors
   * ---------------------------------------------------------------------
   *
   * Check all the non-accessor methods.
   */

  // Check that the object can calculate the correct number of MPDUs
  // needed for a given CSP packet.

  // First do a little CSP config work
  csp_conf_t cspConf;
  csp_conf_get_defaults(&cspConf);
  cspConf.buffer_data_size = 4095; // TODO set as CSP_MTU
  csp_init(&cspConf);

  // Set the CSP packet test lengths so that
  // * a zero length packet is tested
  // * a non-zero length packet fits well into one MPDU
  // * a non-zero length a packet just fits into one MPDU
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numCSPPackets = 5;
  uint16_t cspPacketDataLengths[numCSPPackets] = {0, 10, 103, 358, 4095};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUsPerPacket[numSchemes][numCSPPackets] = {
    {1,1,3,7,71}, // IEEE_802_11N_QCLDPC_648_R_1_2, n = 81 m = 40.5 -> 40 bytes
    {1,1,3,5,53}, // IEEE_802_11N_QCLDPC_648_R_2_3, n = 81 m = 54 bytes
    {1,1,2,5,47}, // IEEE_802_11N_QCLDPC_648_R_3_4, n = 81 m = 60.75 -> 60 bytes
    {1,1,2,5,43}, // IEEE_802_11N_QCLDPC_648_R_5_6, n = 81 m = 67.5 -> 67 bytes
    {2,2,3,7,70}, // IEEE_802_11N_QCLDPC_1296_R_1_2, n = 162 m = 81 bytes
    {2,2,3,6,54}, // IEEE_802_11N_QCLDPC_1296_R_2_3, n = 162 m = 108 bytes
    {2,2,2,6,47}, // IEEE_802_11N_QCLDPC_1296_R_3_4, n = 162 m = 121.5 -> 121 bytes
    {2,2,2,5,43}, // IEEE_802_11N_QCLDPC_1296_R_5_6, n = 162 m = 135 bytes
    {3,3,3,9,70}, // IEEE_802_11N_QCLDPC_1944_R_1_2, n = 243 m = 121.5 -> 121 bytes
    {3,3,3,7,54}, // IEEE_802_11N_QCLDPC_1944_R_2_3, n = 243 m = 162 bytes
    {3,3,3,7,47}, // IEEE_802_11N_QCLDPC_1944_R_3_4, n = 243 m = 182.25 -> 182 bytes
    {3,3,3,5,43}, // IEEE_802_11N_QCLDPC_1944_R_5_6, n = 243 m = 202.5 -> 202 bytes
    {1,1,3,7,70}, // CCSDS_CONVOLUTIONAL_CODING_R_1_2, n = 119 m = 58.75 -> 58 bytes
    {1,1,2,5,53}, // CCSDS_CONVOLUTIONAL_CODING_R_2_3, n = 119 m = 78.5833 -> 78
    {1,1,2,5,47}, // CCSDS_CONVOLUTIONAL_CODING_R_3_4, n = 119 m = 88.5 -> 88 bytes
    {1,1,2,4,42}, // CCSDS_CONVOLUTIONAL_CODING_R_5_6, n = 119 m = 98.4167 -> 98 bytes
    {1,1,2,4,40}, // CCSDS_CONVOLUTIONAL_CODING_R_7_8, n = 119 m = 103.375 -> 103 bytes
    {1,1,1,4,35}  // NO_FEC, m = n = 119
  };
  uint16_t expectedMPDUsPerCodeword[numSchemes] = {
    1, // IEEE_802_11N_QCLDPC_648_R_1_2
    1,   // IEEE_802_11N_QCLDPC_648_R_2_3
    1,   // IEEE_802_11N_QCLDPC_648_R_3_4
    1,   // IEEE_802_11N_QCLDPC_648_R_5_6
    2, // IEEE_802_11N_QCLDPC_1296_R_1_2
    2,   // IEEE_802_11N_QCLDPC_1296_R_2_3
    2,   // IEEE_802_11N_QCLDPC_1296_R_3_4
    2,   // IEEE_802_11N_QCLDPC_1296_R_5_6
    3, // IEEE_802_11N_QCLDPC_1944_R_1_2
    3,   // IEEE_802_11N_QCLDPC_1944_R_2_3
    3,   // IEEE_802_11N_QCLDPC_1944_R_3_4
    3,   // IEEE_802_11N_QCLDPC_1944_R_5_6
    1,   // CCSDS_CONVOLUTIONAL_CODING_R_1_2
    1,   // CCSDS_CONVOLUTIONAL_CODING_R_2_3
    1,   // CCSDS_CONVOLUTIONAL_CODING_R_3_4
    1,   // CCSDS_CONVOLUTIONAL_CODING_R_5_6
    1,   // CCSDS_CONVOLUTIONAL_CODING_R_7_8
    1    // NO_FEC
  };

  ErrorCorrection::ErrorCorrectionScheme ecs;
  ErrorCorrection * errorCorrection;

  for (int ecScheme = 11; ecScheme < 18; ecScheme++) {

    switch(ecScheme) {
      case 0:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2;
        break;
      case 1:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_2_3;
        break;
      case 2:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_3_4;
        break;
      case 3:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_5_6;
        break;
      case 4:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_1_2;
        break;
      case 5:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_2_3;
        break;
      case 6:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_3_4;
        break;
      case 7:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_5_6;
        break;
      case 8:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_1_2;
        break;
      case 9:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_2_3;
        break;
      case 10:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_3_4;
        break;
      case 11:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_5_6;
        break;

      case 12:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        break;
      case 13:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3;
        break;
      case 14:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4;
        break;
      case 15:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6;
        break;
      case 16:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8;
        break;

      case 17:
        ecs = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
        break;
    }

    // Make an MPDUs using several FEC schemes and determine the number
    // of MPDUs needed for the current CSP packet.

    // Error Correction object for the current scheme
    errorCorrection = new ErrorCorrection(ecs, (MPDU::maxMTU() * 8));

    // Get how many MPDUs needed for a codeword
    uint32_t numMPDUsPerCodeword = mpdusPerCodeword(*errorCorrection);
#if QA_MPDU_DEBUG
    printf("numMPDUsPerCordword = %ld\n",numMPDUsPerCodeword);
#endif

    // Check the number of MPDUs per packet required matches expectations
    ASSERT_TRUE(numMPDUsPerCodeword == expectedMPDUsPerCodeword[ecScheme]) << "Incorrect number of MPDUs per codeword " << numMPDUsPerCodeword;

    // Now let's check that the right number of MPDUs are calculated for
    // a number of different CSP packet lengths
    for (uint16_t currLen = 0; currLen < numCSPPackets; currLen++) {

      csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currLen]);

      if (packet == NULL) {
        /* Could not get buffer element */
        csp_log_error("Failed to get CSP buffer");
        FAIL() << "Failed to get CSP buffer";
      }

      // CSP forces us to do our own bookkeeping...
      packet->length = cspPacketDataLengths[currLen];

#if QA_MPDU_DEBUG
      printf("size of packet padding = %ld\n", sizeof(packet->padding));
      printf("size of packet length = %ld\n", sizeof(packet->length));
      printf("size of packet id = %ld\n", sizeof(packet->id));
      // There is no good reason to set the data, but what the heck
      for (unsigned long i = 0; i < cspPacketDataLengths[currLen]; i++) {
        packet->data[i] = (i % 10) | 0x30; // ASCII numbers
      }
#endif

      uint32_t numMPDUsPerPacket = mpdusPerCSPPacket(packet, *errorCorrection);

#if QA_MPDU_DEBUG
      printf("packet length = %d\n", packet->length);
      printf("numMPDUsPerPacket = %d\n", numMPDUsPerPacket);
      printf("expectedMPDUsPerPacket[%d][%d] = %d\n",ecScheme,currLen,expectedMPDUsPerPacket[ecScheme][currLen]);
#endif

      // Clean up!
      csp_buffer_free(packet);

      // Check the number of MPDUs per packet required matches expectations
      ASSERT_TRUE(numMPDUsPerPacket == expectedMPDUsPerPacket[ecScheme][currLen]) << "Incorrect number of MPDUs per CSP Packet " << numMPDUsPerPacket;

    } // for various CSP packet lengths

    delete errorCorrection;

  } // for a number of Error Correction schemes

}


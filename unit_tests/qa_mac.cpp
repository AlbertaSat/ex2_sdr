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

#ifdef __cplusplus
extern "C" {
#endif

#include "csp.h"
#include "csp_types.h"
#include "csp_buffer.h"

#ifdef __cplusplus
}
#endif

#include "mac.hpp"
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
 * @brief Test singleton constructor
 */
TEST(mac, SingletonConstructor)
{
  /* ---------------------------------------------------------------------
   * Check singleton behaviour, then accessors
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  MAC *myMac1 = MAC::instance(modulation, errorCorrectionScheme);

  ASSERT_FALSE(myMac1 == NULL) << "Singleton failed to fetch an instance";

  MAC *myMac2 = MAC::instance(modulation, errorCorrectionScheme);

  ASSERT_TRUE(myMac1 == myMac2) << "Singleton creates more than one instance";

  //
  // Check accessors
  //
  ErrorCorrection::ErrorCorrectionScheme ec = myMac1->getErrorCorrectionScheme();

  ASSERT_TRUE(ec == errorCorrectionScheme) << "Failed to get error correction scheme";

  myMac1->setErrorCorrectionScheme(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2);

  ec = myMac1->getErrorCorrectionScheme();

  ASSERT_TRUE(ec == ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2) << "Failed to set error correction scheme";

  RF_Mode::RF_ModeNumber m = myMac1->getRFModeNumber();

  ASSERT_TRUE(m == modulation) << "Failed to get RF mode number aka modulation";

  myMac1->setRFModeNumber(RF_Mode::RF_ModeNumber::RF_MODE_0);

  m = myMac1->getRFModeNumber();

  ASSERT_TRUE(m == RF_Mode::RF_ModeNumber::RF_MODE_0) << "Failed to set RF mode number aka modulation";
}

/*!
 * @brief Test singleton constructor
 */
TEST(mac, receiveCSPPacket)
{
  /* ---------------------------------------------------------------------
   * Check CSP packet processing
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  MAC *myMac1 = MAC::instance(modulation, errorCorrectionScheme);

  // First do a little CSP config work
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
  uint16_t cspPacketDataLengths[numCSPPackets] = {0, 10, 103, 358, 4096};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUs[numSchemes][numCSPPackets] = {
    {1,1,3,10,101}, // IEEE_802_11N_QCLDPC_648_R_1_2
    {1,1,3,7,77},   // IEEE_802_11N_QCLDPC_648_R_2_3
    {1,1,2,7,68},   // IEEE_802_11N_QCLDPC_648_R_3_4
    {1,1,2,6,61},   // IEEE_802_11N_QCLDPC_648_R_5_6
    {2,2,4,10,102}, // IEEE_802_11N_QCLDPC_1296_R_1_2
    {2,2,4,8,78},   // IEEE_802_11N_QCLDPC_1296_R_2_3
    {2,2,2,8,68},   // IEEE_802_11N_QCLDPC_1296_R_3_4
    {2,2,2,6,62},   // IEEE_802_11N_QCLDPC_1296_R_5_6
    {3,3,3,12,102}, // IEEE_802_11N_QCLDPC_1944_R_1_2
    {3,3,3,9,78},   // IEEE_802_11N_QCLDPC_1944_R_2_3
    {3,3,3,9,69},   // IEEE_802_11N_QCLDPC_1944_R_3_4
    {3,3,3,6,63},   // IEEE_802_11N_QCLDPC_1944_R_5_6
    {1,1,3,7,71},   // CCSDS_CONVOLUTIONAL_CODING_R_1_2
    {1,1,2,5,53},   // CCSDS_CONVOLUTIONAL_CODING_R_2_3
    {1,1,2,5,47},   // CCSDS_CONVOLUTIONAL_CODING_R_3_4
    {1,1,2,4,42},   // CCSDS_CONVOLUTIONAL_CODING_R_5_6
    {1,1,2,4,40},   // CCSDS_CONVOLUTIONAL_CODING_R_7_8
    {1,1,1,4,35}    // NO_FEC
  };

  ErrorCorrection::ErrorCorrectionScheme ecs;
  ErrorCorrection * errorCorrection;

  for (int ecScheme = 0; ecScheme < 1; ecScheme++) {

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

    myMac1->setErrorCorrectionScheme(ecs);
    printf("qa_mac current ECS = %d\n",myMac1->getErrorCorrectionScheme());

    for (uint16_t currentCSPPacket = 0; currentCSPPacket < numCSPPackets; currentCSPPacket++) {

      csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currentCSPPacket]);

      if (packet == NULL) {
        // Could not get buffer element
        csp_log_error("Failed to get CSP buffer");
        FAIL() << "Failed to get CSP buffer";
      }

      // CSP forces us to do our own bookkeeping...
      packet->length = cspPacketDataLengths[currentCSPPacket];
      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
        packet->data[i] = (i % 79) + 0x30; // ASCII numbers through to ~
//        printf("%c",packet->data[i]);
      }
//      printf("\n");

#if QA_MPDU_DEBUG
      printf("size of packet padding = %ld\n", sizeof(packet->padding));
      printf("size of packet length = %ld\n", sizeof(packet->length));
      printf("size of packet id = %ld\n", sizeof(packet->id));
      // There is no good reason to set the data, but what the heck
      for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
        packet->data[i] = (i % 10) | 0x30; // ASCII numbers
      }
#endif

      // It's useful to know how many MPDUs will be produced.
      uint32_t numMPDUs = MPDU::mpdusPerCSPPacket(packet, *errorCorrection);
//printf("numMPDUs = %ld\n",numMPDUs);

      // Process a CSP packet
      bool packetEncoded = myMac1->receiveCSPPacket(packet);

      ASSERT_TRUE(packetEncoded) << "Failed to encode CSP Packet ";

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentCSPPacket]) << "Incorrect number of MPDUs for CSP Packet " << numMPDUs;

      if (packetEncoded) {
//        while (myMac1->nextMPDU())
      }

#if QA_MPDU_DEBUG
      printf("packet length = %d\n", packet->length);
      printf("numMPDUS = %d\n", numMPDUs);
      printf("expectedMPDUs[%d][%d] = %d\n",ecScheme,currentCSPPacket,expectedMPDUs[ecScheme][currentCSPPacket]);
#endif

      // Clean up!
      csp_buffer_free(packet);



    } // for various CSP packet lengths

    delete errorCorrection;

  } // for a number of Error Correction schemes

}

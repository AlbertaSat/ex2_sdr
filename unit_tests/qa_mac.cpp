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
#include "MACWrapper.h"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MAC_DEBUG 0 // set to 1 for debugging output

// @todo should be 14
#define NUM_ERROR_CORRECTION_SCHEMES_TO_TEST 14

/*!
 * @brief Test constructor
 */
TEST(mac, ConstructorAndAccessors)
{
  /* ---------------------------------------------------------------------
   * Make sure objects can be instantiated, then check accessors
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3; // 0b011
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  MAC *myMac1 = new MAC(modulation, errorCorrectionScheme);

  ASSERT_FALSE(myMac1 == NULL) << "Can't instantiate MAC 1";

  MAC *myMac2 = new MAC(modulation, errorCorrectionScheme);

  ASSERT_FALSE(myMac2 == NULL) << "Can't instantiate MAC 2";

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

  delete myMac1;
  delete myMac2;

} // ConstructorsAndAccessors

/*!
 * @brief Test receiving a CSP packet and receiving transparent mode packets
 * when no packets get dropped. Check that the expected number of MPDUs are
 * created for each error correction scheme
 */
TEST(mac, CSPPacketLoopbackNoDroppedPackets) {
  /* ---------------------------------------------------------------------
   * Check CSP packet processing by receiving a CSP packet and then using
   * the resulting MPDUs to emulate received transparent mode packets. Check the
   * correct number of MPDUs are created. Compare the reconstituted CSP packet
   * against the original packet.
   *
   * No transparent mode packets go missing.
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3;
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  // Note, the errorCorrectionScheme is not relevant because it gets changed
  // as we loop through those currently supported.
  MAC *myMac1 = new MAC(modulation, errorCorrectionScheme);

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
  uint16_t cspPacketDataLengths[numCSPPackets] = {0, 10, 103, 358, 4095};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUs[numSchemes][numCSPPackets] = {
    {1,1,1,4,35}, // NO_FEC, m = n = 119
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
    {1,1,3,7,70}, // CCSDS_CONVOLUTIONAL_CODING_R_1_2, n = 118 m = 59 bytes
    {1,1,2,5,53}, // CCSDS_CONVOLUTIONAL_CODING_R_2_3, n = 119 m = 79
    {1,1,2,5,47}, // CCSDS_CONVOLUTIONAL_CODING_R_3_4, n = 119 m = 89 bytes
    {1,1,2,4,42}, // CCSDS_CONVOLUTIONAL_CODING_R_5_6, n = 119 m = 99 bytes
    {1,1,2,4,40}  // CCSDS_CONVOLUTIONAL_CODING_R_7_8, n = 119 m = 104 bytes
  };

  // @note, not all schemes are currently supported; the QCLDPC are stubbed for
  // now, and all but the rate 1/2 convolutional coding schemes are skipped.
  ErrorCorrection::ErrorCorrectionScheme ecs;

  for (int ecScheme = 0; ecScheme < NUM_ERROR_CORRECTION_SCHEMES_TO_TEST; ecScheme++) {

    switch(ecScheme) {
      case 0:
        ecs = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
        break;
      case 1:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2;
        break;
      case 2:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_2_3;
        break;
      case 3:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_3_4;
        break;
      case 4:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_5_6;
        break;
      case 5:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_1_2;
        break;
      case 6:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_2_3;
        break;
      case 7:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_3_4;
        break;
      case 8:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_5_6;
        break;
      case 9:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_1_2;
        break;
      case 10:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_2_3;
        break;
      case 11:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_3_4;
        break;
      case 12:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_5_6;
        break;
      case 13:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        break;
      case 14:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3;
        break;
      case 15:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4;
        break;
      case 16:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6;
        break;
      case 17:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8;
        break;
      default:
        ecs = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
        break;
    }

    // Set the current ECS in the MAC object
    myMac1->setErrorCorrectionScheme(ecs);

    for (uint16_t currentCSPPacket = 0; currentCSPPacket < numCSPPackets; currentCSPPacket++) {

      // Make a CSP packet for the current length

      csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currentCSPPacket]);

      if (packet == NULL) {
        // Could not get buffer element
        csp_log_error("Failed to get CSP buffer");
        FAIL() << "Failed to get CSP buffer";
      }
      // CSP forces us to do our own bookkeeping...
      packet->length = cspPacketDataLengths[currentCSPPacket];
      packet->id.ext = 0x87654321;
      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
        packet->data[i] = (i % 79) + 0x30; // ASCII numbers through to ~
      }

#if QA_MAC_DEBUG
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
      printf("\n");
#endif

      // Process a CSP packet
      bool packetEncoded = myMac1->receiveCSPPacket(packet);

      ASSERT_TRUE(packetEncoded) << "Failed to encode CSP Packet ";

      uint32_t totalPayloadsBytes = myMac1->mpduPayloadsBufferLength();
      uint32_t rawMPDULength = MPDU::rawMPDULength();

      ASSERT_TRUE((totalPayloadsBytes % rawMPDULength) == 0) << "The raw payloads buffer must be an integer multiple of the raw MPDU length.";

      const uint32_t numMPDUs = totalPayloadsBytes / rawMPDULength;
#if QA_MAC_DEBUG
      printf("Raw MPDU length = %ld\n", rawMPDULength);
      printf("totalPayloadsBytes %ld\n",totalPayloadsBytes);
      printf("numMPDUS = %d\n", numMPDUs);
      printf("expectedMPDUs[%d][%d] = %d\n",ecScheme,currentCSPPacket,expectedMPDUs[ecScheme][currentCSPPacket]);
      const uint8_t *mpdusBuff = myMac1->mpduPayloadsBuffer();
      for (uint32_t i = 0; i < totalPayloadsBytes; i++) {
        printf("mpdusBuf[%04d] %02x\n",i,mpdusBuff[i]);
      }
#endif

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentCSPPacket]) << "Incorrect number of MPDUs for CSP Packet " << numMPDUs;

      // At this point we get the mpdu buffer and feed it one raw MPDU at a time
      // into myMac1->processUHFPacket(...)
      if (packetEncoded) {
        const uint8_t *mpdusBuffer = myMac1->mpduPayloadsBuffer();

        if (mpdusBuffer && (myMac1->mpduPayloadsBufferLength() % 128 == 0)) {

          for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
            MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());

            switch (status) {
              case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY:
              {
                const uint8_t *rawCSP = myMac1->getRawCspPacketBuffer();
                uint8_t *p = (uint8_t *) packet;

#if QA_MAC_DEBUG
                printf("packet length %d raw CSP packet length %ld cast CSP packet length %ld\n", packet->length, myMac1->getRawCspPacketBufferLength(), myMac1->getRawCspPacketLength());
                printf("packet length %d (2 bytes) %02x\n", packet->length, packet->length);
                printf("packet id (4 bytes) %04x\n", packet->id);
                for (uint16_t i = 0; i < myMac1->getRawCspPacketLength(); i++) {
                  printf("%04d %02x|%02x\n",i,rawCSP[i],p[i]);
                }
                printf("------------\n");
#endif
                bool same = true;
                for (uint16_t i = 0; same && (i < myMac1->getRawCspPacketLength()); i++) {
                  same = same && (rawCSP[i] == p[i]);
                }
                ASSERT_TRUE(same) << "decoded CSP packet does not match original";
              }
              break;
              case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY_RESUBMIT_PREVIOUS_PACKET:
                break;
              case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
                break;
              default:
                break;
            } // switch on returned UHF packet process status

          } // for all the raw MPDUs
        } // check if have an integral number of raw MPDUs
      } // was the CSP packet successfully encoded

      // Clean up!
      csp_buffer_free(packet);

    } // for various CSP packet lengths

  } // for a number of Error Correction schemes

  delete myMac1;

  csp_free_resources(); // make valgrind happier

} // CSPPacketLoopbackNoDroppedPackets

/*!
 * @brief Test receiving a CSP packet and receiving transparent mode packets
 */
TEST(mac, CSPPacketLoopbackDroppedPackets) {
  /* ---------------------------------------------------------------------
   * Check CSP packet processing by receiving a CSP packet and then using
   * the resulting MPDUs to emulate received transparent mode packets. Check the
   * correct number of MPDUs are created, then lose some to emulate transparent
   * mode packet errors. Compare the reconstituted CSP packet against the
   * original packet. The lengths should match, but the received packet will
   * have different contents.
   *
   * A wrinkle is that if the first MPDU goes missing (is corrupted), then we
   * give up on the CSP packet entirely.
   *
   * We might think that it's not necessary to test all the error correction
   * schemes, but why not?
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3;
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  // Note, the errorCorrectionScheme is not relevant because it gets changed
  // as we loop through those currently supported.
  MAC *myMac1 = new MAC(modulation, errorCorrectionScheme);

  // First do a little CSP config work
  csp_conf_t cspConf;
  csp_conf_get_defaults(&cspConf);
  cspConf.buffer_data_size = 4096; // TODO set as CSP_MTU
  csp_init(&cspConf);

  // Since we want to test what happends when transparent mode packets (MPDUs)
  // go missing, set the CSP packet test lengths so that
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numCSPPackets = 2;
  uint16_t cspPacketDataLengths[numCSPPackets] = {358, 4095};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUs[numSchemes][numCSPPackets] = {
    {4,35}, // NO_FEC, m = n = 119
    {7,71}, // IEEE_802_11N_QCLDPC_648_R_1_2, n = 81 m = 40.5 -> 40 bytes
    {5,53}, // IEEE_802_11N_QCLDPC_648_R_2_3, n = 81 m = 54 bytes
    {5,47}, // IEEE_802_11N_QCLDPC_648_R_3_4, n = 81 m = 60.75 -> 60 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_648_R_5_6, n = 81 m = 67.5 -> 67 bytes
    {7,70}, // IEEE_802_11N_QCLDPC_1296_R_1_2, n = 162 m = 81 bytes
    {6,54}, // IEEE_802_11N_QCLDPC_1296_R_2_3, n = 162 m = 108 bytes
    {6,47}, // IEEE_802_11N_QCLDPC_1296_R_3_4, n = 162 m = 121.5 -> 121 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_1296_R_5_6, n = 162 m = 135 bytes
    {9,70}, // IEEE_802_11N_QCLDPC_1944_R_1_2, n = 243 m = 121.5 -> 121 bytes
    {7,54}, // IEEE_802_11N_QCLDPC_1944_R_2_3, n = 243 m = 162 bytes
    {7,47}, // IEEE_802_11N_QCLDPC_1944_R_3_4, n = 243 m = 182.25 -> 182 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_1944_R_5_6, n = 243 m = 202.5 -> 202 bytes
    {7,70}, // CCSDS_CONVOLUTIONAL_CODING_R_1_2, n = 118 m = 59 bytes
    {5,53}, // CCSDS_CONVOLUTIONAL_CODING_R_2_3, n = 119 m = 79
    {5,47}, // CCSDS_CONVOLUTIONAL_CODING_R_3_4, n = 119 m = 89 bytes
    {4,42}, // CCSDS_CONVOLUTIONAL_CODING_R_5_6, n = 119 m = 99 bytes
    {4,40}  // CCSDS_CONVOLUTIONAL_CODING_R_7_8, n = 119 m = 104 bytes
  };

  // @note, not all schemes are currently supported; the QCLDPC are stubbed for
  // now, and all but the rate 1/2 convolutional coding schemes are skipped.
  ErrorCorrection::ErrorCorrectionScheme ecs;
  std::vector<int> schemes({0});

  for (int ecScheme : schemes) {

    printf("ecs %d\n",ecScheme);
    switch(ecScheme) {
      case 0:
        ecs = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
        break;
      case 1:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2;
        break;
      case 2:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_2_3;
        break;
      case 3:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_3_4;
        break;
      case 4:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_5_6;
        break;
      case 5:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_1_2;
        break;
      case 6:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_2_3;
        break;
      case 7:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_3_4;
        break;
      case 8:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_5_6;
        break;
      case 9:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_1_2;
        break;
      case 10:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_2_3;
        break;
      case 11:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_3_4;
        break;
      case 12:
        ecs = ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_5_6;
        break;
      case 13:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        break;
      case 14:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3;
        break;
      case 15:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4;
        break;
      case 16:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6;
        break;
      case 17:
        ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8;
        break;
      default:
        ecs = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
        break;
    }

    // Set the current ECS in the MAC object
    myMac1->setErrorCorrectionScheme(ecs);

    for (uint16_t currentCSPPacket = 0; currentCSPPacket < numCSPPackets; currentCSPPacket++) {

      // Make a CSP packet for the current length

      csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currentCSPPacket]);

      if (packet == NULL) {
        // Could not get buffer element
        csp_log_error("Failed to get CSP buffer");
        FAIL() << "Failed to get CSP buffer";
      }
      // CSP forces us to do our own bookkeeping...
      packet->length = cspPacketDataLengths[currentCSPPacket];
      packet->id.ext = 0x87654321;
      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
        packet->data[i] = (i % 79) + 0x30; // ASCII numbers through to ~
      }

      // Process a CSP packet
      bool packetEncoded = myMac1->receiveCSPPacket(packet);

      ASSERT_TRUE(packetEncoded) << "Failed to encode CSP Packet ";

      uint32_t totalPayloadsBytes = myMac1->mpduPayloadsBufferLength();
      uint32_t rawMPDULength = MPDU::rawMPDULength();

      // @note we could skip some ASSERTs here since they are done in the happy
      // path unit test above, but it does not hurt to check
      ASSERT_TRUE((totalPayloadsBytes % rawMPDULength) == 0) << "The raw payloads buffer must be an integer multiple of the raw MPDU length.";

      const uint32_t numMPDUs = totalPayloadsBytes / rawMPDULength;

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentCSPPacket]) << "Incorrect number of MPDUs for CSP Packet " << numMPDUs;

      // At this point we get the mpdu buffer and feed it one raw MPDU at a time
      // into myMac1->processUHFPacket(...)
printf("ecs = %d packet = %d\n",ecs,currentCSPPacket);

      if (packetEncoded) {
printf("test missing first MPDU\n");
        const uint8_t *mpdusBuffer = myMac1->mpduPayloadsBuffer();
        if (mpdusBuffer && (myMac1->mpduPayloadsBufferLength() % 128 == 0)) {

          // Test if missing the first MPDU results in a CSP packet. It should not
          for (uint16_t rawMPDUCount = 1; rawMPDUCount < numMPDUs; rawMPDUCount++) {
            MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());

            switch (status) {
              case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY:
              {
                FAIL() << "a CSP packet was made without the first MPDU; this cannot happen";
              }
              break;
              case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY_RESUBMIT_PREVIOUS_PACKET:
                break;
              case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
                break;
              default:
                break;
            } // switch on returned UHF packet process status

          } // for all the raw MPDUs except the first

          // Test if missing the second MPDU results in a CSP packet. It should,
          // and the result should be the correct length, but not match the
          // original
printf("test missing second MPDU\n");
          for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
            // skip the second MPDU
            if (rawMPDUCount != 1) {
              MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
              switch (status) {
                case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY:
                {
                  const uint8_t *rawCSP = myMac1->getRawCspPacketBuffer();
                  uint8_t *p = (uint8_t *) packet;

                  bool same = true;
                  for (uint16_t i = 0; i < myMac1->getRawCspPacketLength(); i++) {
                    same = same && (rawCSP[i] == p[i]);
                  }
                  ASSERT_TRUE(!same) << "decoded CSP packet matches original even though an MPDU was dropped";
                }
                break;
                case MAC::MAC_UHFPacketProcessingStatus::CSP_PACKET_READY_RESUBMIT_PREVIOUS_PACKET:
                  break;
                case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
                  break;
                default:
                  break;
              } // switch on returned UHF packet process status
            } // skip the second MPDU
          } // for all the raw MPDUs except the second


        } // check if have an integral number of raw MPDUs
      } // was the CSP packet successfully encoded

      // Clean up!
      csp_buffer_free(packet);

    } // for various CSP packet lengths

  } // for a number of Error Correction schemes

    delete myMac1;

    csp_free_resources(); // make valgrind happier

} // CSPPacketLoopbackDroppedPackets


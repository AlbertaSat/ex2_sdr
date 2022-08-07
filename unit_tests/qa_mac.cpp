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

#include "mac.hpp"
#include "mpdu.hpp"
#include "mpduHeader.hpp"
#include "MACWrapper.h"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MAC_DEBUG 0         // set to 1 for debugging output
#define QA_MAC_VERBOSE_DEBUG 0 // set to 1 for verbose debugging output

// @todo should be 14
#define NUM_ERROR_CORRECTION_SCHEMES_TO_TEST 14

ErrorCorrection::ErrorCorrectionScheme getScheme(int ecScheme) {
  ErrorCorrection::ErrorCorrectionScheme ecs;
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
  return ecs;
} // getScheme

uint8_t * makePacket(uint16_t length) {
  uint8_t * packet = (uint8_t *) malloc(length);

  for (unsigned long i = 0; i < length; i++) {
    packet[i] = (i % 79) + 0x30; // ASCII numbers through to ~
  }
  return packet;
}

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
 * @brief Test receiving a packet and receiving transparent mode packets
 * when no packets get dropped. Check that the expected number of MPDUs are
 * created for each error correction scheme
 */
TEST(mac, PacketLoopbackNoDroppedPackets) {
  /* ---------------------------------------------------------------------
   * Check packet processing by receiving a  packet and then using the
   * resulting MPDUs to emulate received transparent mode packets. Check the
   * correct number of MPDUs are created. Compare the reconstituted packet
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

  // Set the packet test lengths so that
  // * a zero length packet is tested
  // * a non-zero length packet fits well into one MPDU
  // * a non-zero length a packet just fits into one MPDU
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numPackets = 5;
  size_t packetDataLengths[numPackets] = {0, 10, 119, 358, 4095};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUs[numSchemes][numPackets] = {
    {1,1,1,4,35}, // NO_FEC, m = n = 119
    {1,1,3,7,71}, // IEEE_802_11N_QCLDPC_648_R_1_2, n = 81 m = 40.5 -> 40 bytes
    {1,1,3,5,52}, // IEEE_802_11N_QCLDPC_648_R_2_3, n = 81 m = 54 bytes
    {1,1,2,5,47}, // IEEE_802_11N_QCLDPC_648_R_3_4, n = 81 m = 60.75 -> 60 bytes
    {1,1,2,5,43}, // IEEE_802_11N_QCLDPC_648_R_5_6, n = 81 m = 67.5 -> 67 bytes
    {2,2,3,7,70}, // IEEE_802_11N_QCLDPC_1296_R_1_2, n = 162 m = 81 bytes
    {2,2,3,6,52}, // IEEE_802_11N_QCLDPC_1296_R_2_3, n = 162 m = 108 bytes
    {2,2,2,5,47}, // IEEE_802_11N_QCLDPC_1296_R_3_4, n = 162 m = 121.5 -> 121 bytes
    {2,2,2,5,43}, // IEEE_802_11N_QCLDPC_1296_R_5_6, n = 162 m = 135 bytes
    {3,3,3,7,70}, // IEEE_802_11N_QCLDPC_1944_R_1_2, n = 243 m = 121.5 -> 121 bytes
    {3,3,3,7,54}, // IEEE_802_11N_QCLDPC_1944_R_2_3, n = 243 m = 162 bytes
    {3,3,3,5,47}, // IEEE_802_11N_QCLDPC_1944_R_3_4, n = 243 m = 182.25 -> 182 bytes
    {3,3,3,5,43}, // IEEE_802_11N_QCLDPC_1944_R_5_6, n = 243 m = 202.5 -> 202 bytes
    {1,1,3,7,70}, // CCSDS_CONVOLUTIONAL_CODING_R_1_2, n = 118 m = 59 bytes
    {1,1,2,5,53}, // CCSDS_CONVOLUTIONAL_CODING_R_2_3, n = 119 m = 79
    {1,1,2,5,47}, // CCSDS_CONVOLUTIONAL_CODING_R_3_4, n = 119 m = 89 bytes
    {1,1,2,4,42}, // CCSDS_CONVOLUTIONAL_CODING_R_5_6, n = 119 m = 99 bytes
    {1,1,2,4,40}  // CCSDS_CONVOLUTIONAL_CODING_R_7_8, n = 119 m = 104 bytes
  };

  // @note, not all schemes are currently supported; the QCLDPC are stubbed for
  // now, and all but the rate 1/2 convolutional coding schemes are skipped.

  for (int ecScheme = 0; ecScheme < NUM_ERROR_CORRECTION_SCHEMES_TO_TEST; ecScheme++) {

    ErrorCorrection::ErrorCorrectionScheme ecs  = getScheme(ecScheme);

    // Set the current ECS in the MAC object
    myMac1->setErrorCorrectionScheme(ecs);

    for (uint16_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {

      // Make a user packet for the current length
      uint8_t * packet = makePacket(packetDataLengths[currentPacket]);
      ASSERT_FALSE(packet == NULL) << "Failed to get packet buffer";

      // Process a user packet
      bool packetEncoded = myMac1->receivePacket(packet, packetDataLengths[currentPacket]);
      ASSERT_TRUE(packetEncoded) << "Failed to encode packet ";

      uint32_t totalPayloadsBytes = myMac1->mpduPayloadsBufferLength();
      ASSERT_TRUE((totalPayloadsBytes % MPDU::rawMPDULength()) == 0) << "The raw payloads buffer must be an integer multiple of the raw MPDU length.";

      const uint32_t numMPDUs = totalPayloadsBytes / MPDU::rawMPDULength();
#if QA_MAC_DEBUG
      printf("Raw MPDU length = %d\n", MPDU::rawMPDULength());
      printf("totalPayloadsBytes %d\n",totalPayloadsBytes);
      printf("numMPDUS = %d\n", numMPDUs);
      printf("expectedMPDUs[%d][%d] = %d\n",ecScheme,currentPacket,expectedMPDUs[ecScheme][currentPacket]);
      const uint8_t *mpdusBuff = myMac1->mpduPayloadsBuffer();
      for (uint32_t i = 0; i < totalPayloadsBytes; i++) {
        printf("mpdusBuf[%04d] %02x\n",i,mpdusBuff[i]);
      }
#endif

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentPacket]) << "Incorrect number of MPDUs for Packet " << numMPDUs;

      // At this point we get the mpdu buffer and feed it one raw MPDU at a time
      // into myMac1->processUHFPacket(...)
      const uint8_t *mpdusBuffer = myMac1->mpduPayloadsBuffer();
      ASSERT_FALSE(mpdusBuffer == NULL) << "Failed to get MPDUs buffer";

      bool recvPacketGood = false;
      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());

        switch (status) {
          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
          {
            const uint8_t *rawPacket = myMac1->getRawPacketBuffer();

#if QA_MAC_DEBUG
            printf("original packet length %ld raw packet length %d \n",
              packetDataLengths[currentPacket], myMac1->getRawPacketBufferLength());
            for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
              printf("%04d %02x|%02x\n",i,rawPacket[i],packet[i]);
            }
            printf("------------\n");
#endif
            bool same = true;
            for (uint16_t i = 0; same && (i < myMac1->getRawPacketLength()); i++) {
              same = same && (rawPacket[i] == packet[i]);
            }
            recvPacketGood = same;
            ASSERT_TRUE(same) << "decoded packet does not match original";
          }
          break;
          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
            break;
          default:
            break;
        } // switch on returned UHF packet process status

      } // for all the raw MPDUs

      ASSERT_TRUE(recvPacketGood) << "The expected packet was not received.";


      // Clean up!
      free(packet);

    } // for various packet lengths

  } // for a number of Error Correction schemes

  delete myMac1;

} // PacketLoopbackNoDroppedPackets

/*!
 * @brief Test receiving a packet and receiving transparent mode packets
 */
TEST(mac, PacketLoopbackDroppedPackets) {
  /* ---------------------------------------------------------------------
   * Check packet processing by receiving a packet and then using the
   * resulting MPDUs to emulate received transparent mode packets. Check the
   * correct number of MPDUs are created, then lose some to emulate transparent
   * mode packet errors. Compare the reconstituted packet against the
   * original packet. The lengths should match, but the received packet will
   * have different contents.
   *
   * A wrinkle is that if the first MPDU goes missing (is corrupted), then we
   * give up on the packet entirely.
   *
   * ---------------------------------------------------------------------
   */

  RF_Mode::RF_ModeNumber modulation = RF_Mode::RF_ModeNumber::RF_MODE_3;
  ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme =
      ErrorCorrection::ErrorCorrectionScheme::NO_FEC;

  // Note, the errorCorrectionScheme is not relevant because it gets changed
  // as we loop through those currently supported.
  MAC *myMac1 = new MAC(modulation, errorCorrectionScheme);

  // Since we want to test what happends when transparent mode packets (MPDUs)
  // go missing, set the packet test lengths so that
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numPackets = 2;
  uint16_t packetDataLengths[numPackets] = {358, 4095};

  // Let's choose a few FEC schemes to test; testing them all would take a long
  // time and really we just want to have a mix of n, k, and r.
  // Specifically, let's try the NO_FEC, CCSDS Convolutional Coders, and IEEE
  // 802.11 QCLDPC since they are what we plan to use at a minimum.

  int const numSchemes = 18;
  uint16_t expectedMPDUs[numSchemes][numPackets] = {
    {4,35}, // NO_FEC, m = n = 119
    {7,71}, // IEEE_802_11N_QCLDPC_648_R_1_2, n = 81 m = 40.5 -> 40 bytes
    {5,52}, // IEEE_802_11N_QCLDPC_648_R_2_3, n = 81 m = 54 bytes
    {5,47}, // IEEE_802_11N_QCLDPC_648_R_3_4, n = 81 m = 60.75 -> 60 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_648_R_5_6, n = 81 m = 67.5 -> 67 bytes
    {7,70}, // IEEE_802_11N_QCLDPC_1296_R_1_2, n = 162 m = 81 bytes
    {6,52}, // IEEE_802_11N_QCLDPC_1296_R_2_3, n = 162 m = 108 bytes
    {5,47}, // IEEE_802_11N_QCLDPC_1296_R_3_4, n = 162 m = 121.5 -> 121 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_1296_R_5_6, n = 162 m = 135 bytes
    {7,70}, // IEEE_802_11N_QCLDPC_1944_R_1_2, n = 243 m = 121.5 -> 121 bytes
    {7,54}, // IEEE_802_11N_QCLDPC_1944_R_2_3, n = 243 m = 162 bytes
    {5,47}, // IEEE_802_11N_QCLDPC_1944_R_3_4, n = 243 m = 182.25 -> 182 bytes
    {5,43}, // IEEE_802_11N_QCLDPC_1944_R_5_6, n = 243 m = 202.5 -> 202 bytes
    {7,70}, // CCSDS_CONVOLUTIONAL_CODING_R_1_2, n = 118 m = 59 bytes
    {5,53}, // CCSDS_CONVOLUTIONAL_CODING_R_2_3, n = 119 m = 79
    {5,47}, // CCSDS_CONVOLUTIONAL_CODING_R_3_4, n = 119 m = 89 bytes
    {4,42}, // CCSDS_CONVOLUTIONAL_CODING_R_5_6, n = 119 m = 99 bytes
    {4,40}  // CCSDS_CONVOLUTIONAL_CODING_R_7_8, n = 119 m = 104 bytes
  };

  // @note, not all schemes are currently supported; the QCLDPC are stubbed for
  // now, and all but the rate 1/2 convolutional coding schemes are skipped.
  std::vector<int> schemes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13});

  for (int ecScheme : schemes) {

    ErrorCorrection::ErrorCorrectionScheme ecs  = getScheme(ecScheme);

    // Set the current ECS in the MAC object
    myMac1->setErrorCorrectionScheme(ecs);

    for (uint16_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {

      // Make a user packet for the current length
      uint8_t * packet = makePacket(packetDataLengths[currentPacket]);
      ASSERT_FALSE(packet == NULL) << "Failed to get packet buffer";

      // Process a user packet
      bool packetEncoded = myMac1->receivePacket(packet, packetDataLengths[currentPacket]);
      ASSERT_TRUE(packetEncoded) << "Failed to encode Packet ";

      uint32_t totalPayloadsBytes = myMac1->mpduPayloadsBufferLength();
      // @note we could skip some ASSERTs here since they are done in the happy
      // path unit test above, but it does not hurt to check
      ASSERT_TRUE((totalPayloadsBytes % MPDU::rawMPDULength()) == 0) << "The raw payloads buffer must be an integer multiple of the raw MPDU length.";

      const uint32_t numMPDUs = totalPayloadsBytes / MPDU::rawMPDULength();

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentPacket]) << "Incorrect number of MPDUs for Packet " << numMPDUs;
      ASSERT_TRUE(numMPDUs >= 4) << "All tests need at least 4 MPDUs";

      // At this point we get the mpdu buffer.
      // For the various missing packet scenarios, we feed one or more raw MPDUs
      // into myMac1->processUHFPacket(...)
      const uint8_t *mpdusBuffer = myMac1->mpduPayloadsBuffer();
      ASSERT_FALSE(mpdusBuffer == NULL) << "Failed to get MPDUs buffer";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 1; First raw MPDU is missing, all others received.
      // Expected result; No packet is produced.
      //////////////////////////////////////////////////////////////////////////
#if QA_MAC_VERBOSE_DEBUG
      printf("Scenario 1; First raw MPDU is missing.\n");
#endif
      bool recvPacketGood = false;
      for (uint16_t rawMPDUCount = 1; rawMPDUCount < numMPDUs; rawMPDUCount++) {
        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());

        switch (status) {
          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
          {
            FAIL() << "a packet was made without the first MPDU; this cannot happen";
          }
          break;
          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
            break;
          default:
            break;
        } // switch on returned UHF packet process status

      } // for all the raw MPDUs except the first

      ASSERT_FALSE(recvPacketGood) << "Scenario 1; Even though no first raw MPDU, a packet was received.";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 2; Second raw MPDU is missing, all others received.
      // Expected result; A packet is produced; it will not be correct, but that
      // is to be dealt with by an upper layer. Check the packet does not match
      // the original.
      //////////////////////////////////////////////////////////////////////////
#if QA_MAC_VERBOSE_DEBUG
      printf("Scenario 2; Second raw MPDU is missing.\n");
#endif
      recvPacketGood = false;
      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
        // skip the second MPDU
        if (rawMPDUCount != 1) {
          MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
          switch (status) {
            case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
            {
              const uint8_t *rawPacket = myMac1->getRawPacketBuffer();

              bool same = true;
              for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
                same = same && (rawPacket[i] == packet[i]);
              }
              recvPacketGood = !same;
              ASSERT_FALSE(same) << "Scenario 2; decoded packet matches original even though an MPDU was dropped";
            }
            break;
            case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
              break;
            default:
              break;
          } // switch on returned UHF packet process status
        } // skip the second MPDU
      } // for all the raw MPDUs except the second
      ASSERT_TRUE(recvPacketGood) << "Scenario 2; A packet should have been received even though second raw MPDU was missing.";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 3; Receive the first two raw MPDUs, fragment indices 0 and 1,
      // then receive "new" packet (an MPDU with fragment index 0).
      // Expected result; Only one packet is produced; it will not be correct, but that
      // is to be dealt with by an upper layer. Check the packet does not match
      // the original.
      //////////////////////////////////////////////////////////////////////////
#if QA_MAC_VERBOSE_DEBUG
      printf("Scenario 3; incomplete packet when new packet arrives.\n");
#endif
      recvPacketGood = false;
      for (uint16_t rawMPDUCount = 0; rawMPDUCount < 2; rawMPDUCount++) {
        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());

        switch (status) {
          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
          {
            // It's bad mojo to end up here since we did not provide all the MPDUs.
            recvPacketGood = true;
          }
          break;
          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
            break;
          default:
            break;
        } // switch on returned UHF packet process status

      } // for only the first two raw MPDUs
      ASSERT_FALSE(recvPacketGood) << "Scenario 3; A packet should not been received at this point as only two raw MPDUs have been processed.";
#if QA_MAC_VERBOSE_DEBUG
      printf("first two PMDUs received... now send a new MPDU 0\n");
#endif
      // Now receive another packet.
      recvPacketGood = false;
      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
        switch (status) {
          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
          {
            const uint8_t *rawPacket = myMac1->getRawPacketBuffer();

#if QA_MAC_DEBUG
            printf("original packet length %ld raw packet length %d \n",
              packetDataLengths[currentPacket], myMac1->getRawPacketBufferLength());
            for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
              printf("%04d %02x|%02x\n",i,rawPacket[i],packet[i]);
            }
            printf("------------\n");
#endif
            bool same = true;
            for (uint16_t i = 0; same && (i < myMac1->getRawPacketLength()); i++) {
              same = same && (rawPacket[i] == packet[i]);
            }
            recvPacketGood = same;
#if QA_MAC_VERBOSE_DEBUG
            printf("Scenario 3 packet rec'd\n");
#endif
            ASSERT_TRUE(same) << "Scenario 3; the decoded packet does not match original even though all raw MPDUs received";
          }
          break;
          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
            break;
          default:
            break;
        } // switch on returned UHF packet process status

      } // for all raw MPDUs
      ASSERT_TRUE(recvPacketGood) << "Scenario 3; A packet should have been received.";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 4; Receive raw MPDUs out of order after the first.
      // Expected result; Only one packet is produced; it will not be correct, but that
      // is to be dealt with by an upper layer. Check the packet does not match
      // the original.
      //////////////////////////////////////////////////////////////////////////
#if QA_MAC_VERBOSE_DEBUG
      printf("Scenario 4; Raw MPDUs arrive out of order.\n");
#endif
      // Get the first raw MPDU and process.
      uint16_t rawMPDUCount = 0;
      MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
      switch (status) {
        case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
        {
          FAIL() << "Scenario 4; first MPDU should not result in packet";
        }
        break;
        case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
          break;
        default:
          break;
      } // switch on returned UHF packet process status

      // Get the third raw MPDU and process
      rawMPDUCount = 2;
      status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
      switch (status) {
        case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
        {
          FAIL() << "Scenario 4; third MPDU should not result in packet";
        }
        break;
        case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
          break;
        default:
          break;
      } // switch on returned UHF packet process status

      // Get the second raw MPDU and process. Should put the state machine back
      // looking for the first MPDU, but we can't tell that at this level.
      rawMPDUCount = 1;
      status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
      switch (status) {
        case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
        {
          const uint8_t *rawPacket = myMac1->getRawPacketBuffer();

          bool same = true;
          for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
            same = same && (rawPacket[i] == packet[i]);
          }
          recvPacketGood = same;
          ASSERT_FALSE(same) << "Scenario 4; decoded packet matches original even though MPDUs were out of order";
        }
        break;
        case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
        {
          FAIL() << "Scenario 4; second (out of order) MPDU should result in packet";
        }
          break;
        default:
          break;
      } // switch on returned UHF packet process status

      // Finally, get the last raw MPDU and process. Since we expect the state
      // machine to have been put back to looking for the first MPDU, providing
      // the final MPDU should not result in a packet.
      rawMPDUCount = numMPDUs - 1;
      status = myMac1->processUHFPacket(mpdusBuffer+rawMPDUCount*MPDU::rawMPDULength(), MPDU::rawMPDULength());
      switch (status) {
        case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
        {
          FAIL() << "Scenario 4; final MPDU should not result in packet";
        }
        break;
        case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
          break;
        default:
          break;
      } // switch on returned UHF packet process status
      ASSERT_FALSE(recvPacketGood) << "Scenario 4; A corrupted packet should have been received.";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 5a); Receive raw MPDUs in order, but corrupt the MPDU header
      // first Golay-encoded message (12 bits).
      // Expected result; No packet is produced because the first MPDU is
      // corrupted and the rest of the MPDUs are dropped.
      //////////////////////////////////////////////////////////////////////////
//#if QA_MAC_VERBOSE_DEBUG
//      printf("Scenario 5a; Error correction scheme in MPDU header bits is corrupted.\n");
//#endif
//      recvPacketGood = false;
//      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
//
//        // Make a copy fo the current MPDU because we might use the original in a later test
//        std::vector<uint8_t> rawMPDU;
//        for (uint16_t i = 0; i < MPDU::rawMPDULength(); i++) {
//          rawMPDU.push_back(mpdusBuffer[rawMPDUCount*MPDU::rawMPDULength()+i]);
//        }
//
//        // Now, let's mangle the 5 MSBs of the 6-bit MPDUHeader error correction
//        // enum, but only for the first MPDU
//        if (rawMPDUCount == 0) {
//#if QA_MAC_VERBOSE_DEBUG
//          printf("Scenario 5a) rawMPDU[0] = 0x%02x ",rawMPDU[0]);
//#endif
//          uint8_t temp = ~rawMPDU[0];
//          temp = (temp & 0x1F) | (rawMPDU[0] & 0xE0);
//          rawMPDU[0] = temp;
//#if QA_MAC_VERBOSE_DEBUG
//          printf("temp = 0x%02x\n",temp);
//#endif
//        }
//        try {
//        // process the copy
//        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(&rawMPDU[0], MPDU::rawMPDULength());
//
//        switch (status) {
//          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
//          {
//            recvPacketGood = true;
//            ASSERT_FALSE(recvPacketGood) << "Scenario 5a; A packet was decoded, which should not happen";
//          }
//          break;
//          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
//            break;
//          default:
//            break;
//        } // switch on returned UHF packet process status
//        }
//        catch (const MPDUException& e) {
//
//        }
//
//      } // for all raw MPDUs
//      ASSERT_FALSE(recvPacketGood) << "Scenario 5a; A packet should not been received at this point";
//
//      //////////////////////////////////////////////////////////////////////////
//      // Scenario 5b); Receive raw MPDUs in order, but corrupt the MPDU header
//      // second Golay-encoded message (12 bits).
//      // Expected result; One packet is produced; the user packet length field
//      // is corrupted in the third MPDU, so the rest of the packet is padded
//      // and then decoded.
//      //////////////////////////////////////////////////////////////////////////
//#if QA_MAC_VERBOSE_DEBUG
//      printf("Scenario 5b; Uaser packet length in MPDU header bits is corrupted.\n");
//#endif
//      recvPacketGood = false;
//      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {
//
//        // Make a copy fo the current MPDU because we might use the original in a later test
//        std::vector<uint8_t> rawMPDU;
//        for (uint16_t i = 0; i < MPDU::rawMPDULength(); i++) {
//          rawMPDU.push_back(mpdusBuffer[rawMPDUCount*MPDU::rawMPDULength()+i]);
//        }
//
//        // Now, let's mangle the 7 LSBs of the user packet length field in the
//        // 3rd MPDU. Since the Golay-encoded codeword is 24 bits == 3 bytes, we
//        // need to mangle the 7 LSBs of the 5th byte in the MPDU
//        if (rawMPDUCount == 2) {
//#if QA_MAC_VERBOSE_DEBUG
//          printf("Scenario 5b) rawMPDU[4] = 0x%02x ",rawMPDU[4]);
//#endif
//          uint8_t temp = ~rawMPDU[4];
//          temp = (temp & 0x7F) | (rawMPDU[4] & 0x80);
//          rawMPDU[4] = temp;
//#if QA_MAC_VERBOSE_DEBUG
//          printf("temp = 0x%02x\n",rawMPDU[4]);
//#endif
//        }
//        try {
//        // process the copy
//        MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(&rawMPDU[0], MPDU::rawMPDULength());
//
//        switch (status) {
//          case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
//          {
//            const uint8_t *rawPacket = myMac1->getRawPacketBuffer();
//
//            bool same = true;
//            for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
//              same = same && (rawPacket[i] == packet[i]);
//            }
//            recvPacketGood = same;
//            ASSERT_FALSE(recvPacketGood) << "Scenario 5b; A packet was decoded with no errors, which should not happen";
//          }
//          break;
//          case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
//            break;
//          default:
//            break;
//        } // switch on returned UHF packet process status
//        }
//        catch (const MPDUException& e) {
//
//        }
//
//      } // for all raw MPDUs
//      ASSERT_FALSE(recvPacketGood) << "Scenario 5b; A good packet should not been received at this point";

      //////////////////////////////////////////////////////////////////////////
      // Scenario 5c); Receive raw MPDUs in order, but corrupt the MPDU header
      // third Golay-encoded message (12 bits).
      // Expected result; One packet is produced; the user packet fragment index
      // is corrupted in the third MPDU, so the rest of the packet is padded
      // and then decoded.
      //////////////////////////////////////////////////////////////////////////
#if QA_MAC_VERBOSE_DEBUG
      printf("Scenario 5c; User packet fragment index in MPDU header bits is corrupted.\n");
#endif
      recvPacketGood = false;
      for (uint16_t rawMPDUCount = 0; rawMPDUCount < numMPDUs; rawMPDUCount++) {

        // Make a copy fo the current MPDU because we might use the original in a later test
        std::vector<uint8_t> rawMPDU;
        for (uint16_t i = 0; i < MPDU::rawMPDULength(); i++) {
          rawMPDU.push_back(mpdusBuffer[rawMPDUCount*MPDU::rawMPDULength()+i]);
        }

        // Now, let's corrupt 7 bits of the user packet fragment index in the
        // 3rd MPDU. Since the Golay-encoded codeword is 24 bits == 3 bytes, we
        // need to mangle the LSB of the 9th byte in the MPDU
        if (rawMPDUCount == 2) {
#if QA_MAC_VERBOSE_DEBUG
          for (uint16_t j=0; j < 9; j++) {
            printf("0x%02X ",rawMPDU[j]);
          }
          printf("\n");
          printf("Scenario 5c) rawMPDU[8] = 0x%02x ",rawMPDU[8]);
#endif
          uint8_t temp = (~rawMPDU[8]) & 0xFE;
          rawMPDU[8] = rawMPDU[8] | temp;
#if QA_MAC_VERBOSE_DEBUG
          printf("temp = 0x%02x\n",rawMPDU[8]);
#endif
        }
        try {
          // process the copy
          MAC::MAC_UHFPacketProcessingStatus status = myMac1->processUHFPacket(&rawMPDU[0], MPDU::rawMPDULength());

          switch (status) {
            case MAC::MAC_UHFPacketProcessingStatus::PACKET_READY:
            {
              const uint8_t *rawPacket = myMac1->getRawPacketBuffer();

              bool same = true;
              for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
                same = same && (rawPacket[i] == packet[i]);
              }
              recvPacketGood = same;
              ASSERT_FALSE(recvPacketGood) << "Scenario 5c; A packet was decoded with no errors, which should not happen";
            }
            break;
            case MAC::MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET:
              break;
            default:
              break;
          } // switch on returned UHF packet process status
        }
        catch (const MPDUException& e) {

        }

      } // for all raw MPDUs
      ASSERT_FALSE(recvPacketGood) << "Scenario 5c; A good packet should not been received at this point";

      // Clean up!
      free(packet);

    } // for various packet lengths

  } // for a number of Error Correction schemes

  delete myMac1;

} // PacketLoopbackDroppedPackets


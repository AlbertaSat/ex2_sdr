/*!
 * @file qa_macWrapper.cpp
 * @author Steven Knudsen
 * @date June 6, 2021
 *
 * @details Unit test for the MAC wrapper class.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "MACWrapper.h"
#include "error_correctionWrapper.h"
#include "rfModeWrapper.h"

#include <gtest/gtest.h>

#define QA_MAC_WRAPPER_DEBUG 0 // set to 1 for debugging output

#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // UHF transparent mode packet is always 128 bytes


TEST(macWrapper, ConstructorAndAccessors_wrapper) {
  /* ---------------------------------------------------------------------
   * Make sure objects can be instantiated, then check accessors
   * ---------------------------------------------------------------------
   */

  error_correction_scheme_t errorCorrectionScheme = NO_FEC;
  rf_mode_number_t rfMode = RF_MODE_3;

  mac_t *myMac1 = mac_create(rfMode, errorCorrectionScheme);

  ASSERT_FALSE(myMac1 == NULL) << "Can't instantiate MAC 1";

  mac_t *myMac2 = mac_create(rfMode, errorCorrectionScheme);

  ASSERT_FALSE(myMac2 == NULL) << "Can't instantiate MAC 2";

  //
  // Check accessors
  //
  error_correction_scheme_t ec = get_error_correction_scheme(myMac1);

  ASSERT_TRUE(ec == errorCorrectionScheme) << "Failed to get error correction scheme";

  set_error_correction_scheme(myMac1, CCSDS_CONVOLUTIONAL_CODING_R_1_2);

  ec = get_error_correction_scheme(myMac1);

  ASSERT_TRUE(ec == CCSDS_CONVOLUTIONAL_CODING_R_1_2) << "Failed to set error correction scheme";

  rf_mode_number_t m = get_rf_mode_number(myMac1);

  ASSERT_TRUE(m == rfMode) << "Failed to get RF mode number aka modulation";

  set_rf_mode_number(myMac1, RF_MODE_0);

  m = get_rf_mode_number(myMac1);

  ASSERT_TRUE(m == RF_MODE_0) << "Failed to set RF mode number aka modulation";

  //
  // Check destructor
  //

  mac_destroy(myMac1);

// @todo there is no reliable way to tell if a C pointer has been freed. I did
// check by calling one of the accessors again using myMac1 and that seg faulted,
// so I am pretty sure the mac_destroy method works
//  ASSERT_TRUE(myMac1 == NULL) << "Can't destroy MAC 1";

  mac_destroy(myMac2);

//  ASSERT_TRUE(myMac2 == NULL) << "Can't destroy MAC 2";

} // ConstructorsAndAccessors

/*!
 * @brief Test receiving a packet and receiving transparent mode packets
 */
TEST(macWrapper, PacketLoopbackNoDroppedPackets) {
  /* ---------------------------------------------------------------------
   * Same as the PacketLoopbackNoDroppedPackets test, but using the wrapper.
   *
   * Check packet processing by receiving a packet and then using
   * the resulting MPDUs to emulate received transparent mode packets.
   * Compare the reconstituted packet against the original packet
   * ---------------------------------------------------------------------
   */
  rf_mode_number_t rd_mode = RF_MODE_3; // 0b011
  error_correction_scheme_t error_correction_scheme = NO_FEC;

  mac_t *myMac1 = mac_create(rd_mode, error_correction_scheme);

  ASSERT_FALSE(myMac1 == NULL) << "Can't instantiate MAC 1";

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

  error_correction_scheme_t ecs;

  for (int ecScheme = 0; ecScheme < 14; ecScheme++) {

    switch(ecScheme) {
      case 0:
        ecs = NO_FEC;
        break;
      case 1:
        ecs = IEEE_802_11N_QCLDPC_648_R_1_2;
        break;
      case 2:
        ecs = IEEE_802_11N_QCLDPC_648_R_2_3;
        break;
      case 3:
        ecs = IEEE_802_11N_QCLDPC_648_R_3_4;
        break;
      case 4:
        ecs = IEEE_802_11N_QCLDPC_648_R_5_6;
        break;
      case 5:
        ecs = IEEE_802_11N_QCLDPC_1296_R_1_2;
        break;
      case 6:
        ecs = IEEE_802_11N_QCLDPC_1296_R_2_3;
        break;
      case 7:
        ecs = IEEE_802_11N_QCLDPC_1296_R_3_4;
        break;
      case 8:
        ecs = IEEE_802_11N_QCLDPC_1296_R_5_6;
        break;
      case 9:
        ecs = IEEE_802_11N_QCLDPC_1944_R_1_2;
        break;
      case 10:
        ecs = IEEE_802_11N_QCLDPC_1944_R_2_3;
        break;
      case 11:
        ecs = IEEE_802_11N_QCLDPC_1944_R_3_4;
        break;
      case 12:
        ecs = IEEE_802_11N_QCLDPC_1944_R_5_6;
        break;
      case 13:
        ecs = CCSDS_CONVOLUTIONAL_CODING_R_1_2;
        break;
      case 14:
        ecs = CCSDS_CONVOLUTIONAL_CODING_R_2_3;
        break;
      case 15:
        ecs = CCSDS_CONVOLUTIONAL_CODING_R_3_4;
        break;
      case 16:
        ecs = CCSDS_CONVOLUTIONAL_CODING_R_5_6;
        break;
      case 17:
        ecs = CCSDS_CONVOLUTIONAL_CODING_R_7_8;
        break;
    }

    // Make an MPDUs using several FEC schemes and determine the number
    // of MPDUs needed for the current packet.

    set_error_correction_scheme(myMac1, ecs);

    for (uint16_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {

      uint8_t * packet = (uint8_t *) malloc(packetDataLengths[currentPacket]);

      if (packet == NULL) {
        FAIL() << "Failed to get buffer";
      }

      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < packetDataLengths[currentPacket]; i++) {
        packet[i] = (i % 79) + 0x30; // ASCII numbers through to ~
      }

      // Process a packet
      bool packetEncoded = prepare_packet_for_tx(myMac1, packet, packetDataLengths[currentPacket]);

      ASSERT_TRUE(packetEncoded) << "Failed to encode Packet ";

      uint32_t totalPayloadsBytes = mpdu_payloads_buffer_length(myMac1);
      uint32_t rawMPDULength = raw_mpdu_length();

      ASSERT_TRUE((totalPayloadsBytes % rawMPDULength) == 0) << "The raw payloads buffer must be an integer multiple of the raw MPDU length.";

      uint32_t numMPDUs = totalPayloadsBytes / rawMPDULength;
#if QA_MAC_WRAPPER_DEBUG
      printf("Raw MPDU length = %d\n", rawMPDULength);
      printf("totalPayloadsBytes %d\n",totalPayloadsBytes);
      printf("numMPDUS = %d\n", numMPDUs);
      printf("expectedMPDUs[%d][%d] = %d\n",ecScheme,currentPacket,expectedMPDUs[ecScheme][currentPacket]);
//      const uint8_t *mpdusBuff = myMac1->mpduPayloadsBuffer();
//      for (uint32_t i = 0; i < totalPayloadsBytes; i++) {
//        printf("mpdusBuf[%04d] %02x\n",i,mpdusBuff[i]);
//      }
#endif

      // Check the number of MPDUs required matches expectations
      ASSERT_TRUE(numMPDUs == expectedMPDUs[ecScheme][currentPacket]) << "Incorrect number of MPDUs for packet " << numMPDUs;

      // At this point we could get the mpdu buffer and feed it one raw MPDU
      // at a time into myMac1->processUHFPacket(...)
      if (packetEncoded) {
        const uint8_t *mpdusBuffer = mpdu_payloads_buffer(myMac1);

        // @todo get rid of magic number
        if (mpdusBuffer && (mpdu_payloads_buffer_length(myMac1) % 128 == 0)) {

          for (uint16_t rawMPDUIndex = 0; rawMPDUIndex < mpdu_payloads_buffer_length(myMac1); rawMPDUIndex += 128) {
            packet_processing_status_t status = process_packet(myMac1, mpdusBuffer+rawMPDUIndex, 128);
            switch (status) {
              case PACKET_READY:
              {
                const uint8_t *rawPacket = get_raw_packet_buffer(myMac1);

#if QA_MAC_WRAPPER_DEBUG
//                printf("original packet length %ld raw packet length %d \n",
//                  packetDataLengths[currentPacket], myMac1->getRawPacketBufferLength());
//                for (uint16_t i = 0; i < myMac1->getRawPacketLength(); i++) {
//                  printf("%04d %02x|%02x\n",i,rawPacket[i],packet[i]);
//                }
//                printf("------------\n");
#endif
                bool same = true;
                for (uint16_t i = 0; i < get_raw_packet_length(myMac1); i++) {
                  same = same && (rawPacket[i] == packet[i]);
                }
                ASSERT_TRUE(same) << "decoded packet does not match original";
              }
              break;
              case PACKET_READY_RESUBMIT_PREVIOUS_PACKET:
                break;
              case READY_FOR_NEXT_PACKET:
                break;
              default:
                break;
            } // switch on returned packet process status

          } // for all the raw MPDUs
        } // check if have an integral number of raw MPDUs
      } // was the packet successfully encoded

      // Clean up!
      free(packet);

    } // for various packet lengths

  } // for a number of Error Correction schemes

  mac_destroy(myMac1);

} // PacketLoopback_wrapper


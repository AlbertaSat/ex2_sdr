/*!
 * @file qa_convCode27.cpp
 * @author Arash Yazdani
 * @date August 5, 2021
 *
 * @details Unit test for the Convolutional Code with rate=1/2 & K=7.
 *
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
#include "convCode27.hpp"
#include "FEC.hpp"
#include "mpdu.hpp"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_CC27_DEBUG 1 // set to 1 for debugging output

//#define UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH ( 72/8 ) // bytes
//#define UHF_TRANSPARENT_MODE_PACKET_LENGTH ( 128 )         // bytes; UHF transparent mode packet is always 128 bytes
//#define UHF_TRANSPARENT_MODE_PACKET_PAYLOAD_LENGTH ( UHF_TRANSPARENT_MODE_PACKET_LENGTH - UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH )
/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(CC27_1_2, EncodeAndDecode )
{
  /* ---------------------------------------------------------------------
   * Confirm the operation of the CCSDS_CONVOLUTIONAL_CODING_R_1_2 FEC
   * ---------------------------------------------------------------------
   */

  // Set the packet test lengths so that
  // * a zero length packet is tested
  // * a non-zero length packet fits well into one MPDU
  // * a non-zero length a packet just fits into one MPDU
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numPackets = 5;
  uint16_t packetDataLengths[numPackets] = {0, 10, 119, 358, 4095};

  ErrorCorrection::ErrorCorrectionScheme ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
  ErrorCorrection * errorCorrection = new ErrorCorrection(ecs, (MPDU::maxMTU() * 8));
  FEC *CC27_FEC = FEC::makeFECCodec(ecs);

  ASSERT_TRUE(CC27_FEC != NULL) << "CCSDS_CONVOLUTIONAL_CODING_R_1_2 FEC failed to instantiate";

  std::vector<uint8_t> packet;

  for (uint16_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {

    packet.resize(0);

    // Set the payload to readable ASCII
    for (unsigned long i = 0; i < packetDataLengths[currentPacket]; i++) {
      packet.push_back( (i % 79) + 0x30 ); // ASCII numbers through to ~
    }


    /***********************************************
     * TODO WIP ************************************
     * *********************************************
     */
    // @note the message length returned by the ErrorCorrection object is
    // in bits. It may be that it's not a multiple of 8 bits (1 byte), so
    // we truncate the length and assume the encoder pads the message with
    // zeros for the missing bits
    uint32_t const messageLength = errorCorrection->getMessageLen() / 8;

  } // for various packet lengths

} // CC27_1_2 EncodeAndDecode


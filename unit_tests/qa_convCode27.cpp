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

  // Set the CSP packet test lengths so that
  // * a zero length packet is tested
  // * a non-zero length packet fits well into one MPDU
  // * a non-zero length a packet just fits into one MPDU
  // * a non-zero length a packet needs more than one MPDU
  // * the max size packet
  uint16_t const numCSPPackets = 5;
  uint16_t cspPacketDataLengths[numCSPPackets] = {0, 10, 103, 358, 4095};

  ErrorCorrection::ErrorCorrectionScheme ecs = ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2;
  ErrorCorrection * errorCorrection = new ErrorCorrection(ecs, (MPDU::maxMTU() * 8));
  FEC *CC27_FEC = FEC::makeFECCodec(ecs);

  ASSERT_TRUE(CC27_FEC != NULL) << "CCSDS_CONVOLUTIONAL_CODING_R_1_2 FEC failed to instantiate";

  //
  // This test does not have to use CSP packets necessarily, but it provides a
  // little more reassurance if we can see actual CSP data being worked on.
  //
  csp_conf_t cspConf;
  csp_conf_get_defaults(&cspConf);
  cspConf.buffer_data_size = 4096; // TODO set as CSP_MTU
  csp_init(&cspConf);

  for (uint16_t currentCSPPacket = 0; currentCSPPacket < numCSPPackets; currentCSPPacket++) {


    csp_packet_t * cspPacket = (csp_packet_t *) csp_buffer_get(cspPacketDataLengths[currentCSPPacket]);

    if (cspPacket == NULL) {
      // Could not get buffer element
      csp_log_error("Failed to get CSP buffer");
      FAIL() << "Failed to get CSP buffer";
    }
    // CSP forces us to do our own bookkeeping...
    cspPacket->length = cspPacketDataLengths[currentCSPPacket];
    cspPacket->id.ext = 0x87654321;
    // Set the payload to readable ASCII
    for (unsigned long i = 0; i < cspPacketDataLengths[currentCSPPacket]; i++) {
      cspPacket->data[i] = (i % 79) + 0x30; // ASCII numbers through to ~
    }

#if QA_CC27_DEBUG
    printf("size of packet padding = %ld\n", sizeof(cspPacket->padding));
    printf("size of packet length = %ld\n", sizeof(cspPacket->length));
    printf("size of packet id = %ld\n", sizeof(cspPacket->id));
    printf("size of csp_id_t %ld\n",sizeof(csp_id_t));
    printf("packet length %d (2 bytes) %02x\n", cspPacket->length, cspPacket->length);
    printf("packet id (4 bytes) %04x\n", cspPacket->id);
    printf("Padding\n\t");
    for (uint8_t p = 0; p < sizeof(cspPacket->padding); p++) {
      printf("%02x",cspPacket->padding[p]);
    }
    printf("\n");
#endif

    uint16_t const cspPacketLength = sizeof(csp_packet_t) + cspPacket->length;

    // @note the message length returned by the ErrorCorrection object is
    // in bits. It may be that it's not a multiple of 8 bits (1 byte), so
    // we truncate the length and assume the encoder pads the message with
    // zeros for the missing bits
    uint32_t const messageLength = errorCorrection->getMessageLen() / 8;

    // Keep track of how much CSP packet data has been encoded
    uint32_t cspDataOffset = 0;
    uint32_t cspBytesRemaining = cspPacketLength;

    // Set up the MPDU payload
    PPDU_u8::payload_t mpduPayload;
    mpduPayload.resize(0); // ensure it's empty
    uint32_t mpduPayloadBytesRemaining = MPDU::maxMTU();
    uint32_t mpduCount = 0;

    PPDU_u8::payload_t message;

    message.resize(0);
    message.insert(message.end(), cspPacket->padding, cspPacket->padding + CSP_PADDING_BYTES);
    cspBytesRemaining -= CSP_PADDING_BYTES;
    message.push_back((uint8_t) (cspPacket->length & 0x00FF));
    message.push_back((uint8_t) ((cspPacket->length & 0xFF00) >> 8));
    cspBytesRemaining -= sizeof(uint16_t);
    message.push_back((uint8_t) (cspPacket->id.ext & 0x000000FF));
    message.push_back((uint8_t) ((cspPacket->id.ext & 0x0000FF00) >> 8));
    message.push_back((uint8_t) ((cspPacket->id.ext & 0x00FF0000) >> 16));
    message.push_back((uint8_t) ((cspPacket->id.ext & 0xFF000000) >> 24));
    cspBytesRemaining -= sizeof(uint32_t);


    // We know/assume that the CSP header is smaller than the smallest message
    // size for any FEC scheme we employ, so we need to do at least one
    // iteration of this loop
    do {
      // Fill the rest of the message buffer with CSP data; if not enough CSP data is
      // available, use what remains and pad
      if (message.size() < messageLength) {
        // Check if we can fill the rest of the message
        if (cspBytesRemaining >= messageLength - message.size()) {
          //            printf("no padding\n");
          // More than enough CSP packet data remaining, so fill up the message
          uint32_t bytesToAppend = messageLength - message.size();
          message.insert(message.end(),
            cspPacket->data + cspDataOffset, cspPacket->data + cspDataOffset + messageLength - message.size());
          cspBytesRemaining -= bytesToAppend;
          cspDataOffset += bytesToAppend;
        }
        else {
          // Not enough CSP packet data remaining, so put what there is in message
          message.insert(message.end(),
            cspPacket->data + cspDataOffset, cspPacket->data + cspDataOffset + cspBytesRemaining);
          cspDataOffset += cspBytesRemaining; // @todo don't really need to update this
          cspBytesRemaining -= cspBytesRemaining;
          // Zero-pad the rest of the message
          message.resize(messageLength, 0);
        }
      }

      // Now apply the FEC encoding
      PPDU_u8 chunk(message);
      try {
        // @todo, the m_FEC should always exist... this code was needed duing dev and test and could be removed
        if (!CC27_FEC) {
          printf("CC27_FEC bad\n");
          FAIL( ) << "CC27_FEC bad";
        }
        PPDU_u8 encodedChunk = CC27_FEC->encode(chunk);

        // Add codeword to current mpduPayload
        PPDU_u8::payload_t codeword = encodedChunk.getPayload();

#if QA_CC27_DEBUG
        printf("cw.size() %ldb codeword size %ldb\n", codeword.size()*8, errorCorrection->getCodewordLen());
        printf("cw.size() %ldB codeword size %ldB\n", codeword.size(), errorCorrection->getCodewordLen() / 8);
        printf("byte message codeword\n");
        for (uint16_t c = 0; c < codeword.size(); c++) {
          printf("%04d  %02x     %02x\n", c, message[c], codeword[c]);
        }
#endif
        ASSERT_TRUE(codeword.size() == errorCorrection->getCodewordLen() / 8) << "CC27 codeword size incorrect";

        PPDU_u8::payload_t decodedMessage;

//        __attribute__((unused)) uint32_t bitErrors = CC27_FEC->decode(codeword, 100.0, decodedMessage);
        uint32_t bitErrors = CC27_FEC->decode(codeword, 100.0, decodedMessage);

#if QA_CC27_DEBUG
        printf("message %ld decodedMessage %ld\n", message.size(), decodedMessage.size());
        printf("byte message decodedMessage\n");
#endif

//        ASSERT_TRUE(message.size() == decodedMessage.size()) << "Decoded message different length than message.";

        bool same = true;
        for (uint16_t m = 0; m < decodedMessage.size(); m++) {
          same = same && (message[m] == decodedMessage[m]);
#if QA_CC27_DEBUG
        printf("%04d  %02x      %02x\n", m, message[m], decodedMessage[m]);
#endif
        }

        // prepare to make another message
        message.resize(0);
      }
      catch (FECException& e) { // @todo need an FEC exception that all subclasses inherit
      }

    } while (cspBytesRemaining > 0);


    // TODO: Add noise to the encoded symbols and see how many can decoder correct.

  } // for various CSP packet lengths
} // CC27_1_2 EncodeAndDecode


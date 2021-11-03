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

#include "convCode27.hpp"

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

#define QA_CC27_DEBUG 0 // set to 1 for debugging output

#define UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH ( 72/8 ) // bytes
#define UHF_TRANSPARENT_MODE_PACKET_LENGTH ( 128 )         // bytes; UHF transparent mode packet is always 128 bytes
#define UHF_TRANSPARENT_MODE_PACKET_PAYLOAD_LENGTH ( UHF_TRANSPARENT_MODE_PACKET_LENGTH - UHF_TRANSPARENT_MODE_PACKET_HEADER_LENGTH )
/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(CC27, Foo )
{
  /* ---------------------------------------------------------------------
   * Confirm the CC27 object can be constructed
   * ---------------------------------------------------------------------
   */

  // First do a little CSP config work
  csp_conf_t cspConf;
  csp_conf_get_defaults(&cspConf);
  cspConf.buffer_data_size = 4096; // TODO set as CSP_MTU
  csp_init(&cspConf);

  // Set the length of the test CSP packet so it all fits into a transparent mode payload
  const unsigned long int testCSPPacketLength = 10;

  FEC * CC27 = new convCode27(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2);

  ASSERT_TRUE(CC27 != NULL) << "convCode27 failed to instantiate";

  if (CC27) {
    csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(testCSPPacketLength);

    if (packet == NULL) {
      /* Could not get buffer element */
      csp_log_error("Failed to get CSP buffer");
      return;
    }

//    printf("size of packet padding = %ld\n", sizeof(packet->padding));
//    printf("size of packet length = %ld\n", sizeof(packet->length));
//    printf("size of packet id = %ld\n", sizeof(packet->id));
    int cspPacketHeaderLen = sizeof(packet->padding) + sizeof(packet->length) + sizeof(packet->id);

    for (unsigned long i = 0; i < testCSPPacketLength; i++) {
      packet->data[i] = i | 0x30; // ASCII numbers
    }
    // CSP forces us to do our own bookkeeping...
    packet->length = testCSPPacketLength;

//    printf("packet length = %d\n", packet->length);

    std::vector<uint8_t> p;

    uint8_t * pptr = (uint8_t *) packet;
    for (int i = 0; i < cspPacketHeaderLen; i++) {
      p.push_back(pptr[i]);
    }
    // This is ugly, so maybe we need to rethink using PPDU_xx?
    for (unsigned long i = 0; i < testCSPPacketLength; i++) {
      p.push_back(packet->data[i]);
    }

//    // Look at the contents :-)
//    for (int i = 0; i < p.size(); i++) {
//      printf("p[%d] = 0x%02x\n", i, p[i]);
//    }

    // @TODO maybe make these std::vector<uint8_t> ???
    PPDU_u8 inputPayload(p);
    PPDU_u8 encodedPayload = CC27->encode(inputPayload);

    bool same = true;
    std::vector<uint8_t> iPayload = inputPayload.getPayload();
    std::vector<uint8_t> ePayload = encodedPayload.getPayload();

    // Noise-free channel to check if the algorithms are working correctly
    std::vector<uint8_t> dPayload;
    uint32_t bitErrors = CC27->decode(encodedPayload.getPayload(), 100.0, dPayload);

    printf("ipayload size %ld dpayload size %ld\n",iPayload.size(),dPayload.size());
    ASSERT_TRUE(iPayload.size() == dPayload.size()) << "Encoded and decoded payload lengths differ!";
    same = true;
    for (unsigned long i = 0; i < iPayload.size(); i++) {
      same = same & (iPayload[i] == dPayload[i]);
      #if QA_CC27_DEBUG
      printf("input[%d] = 0x%02x    encoded[%d] = 0x%02x    decoded[%d] = 0x%02x\n", i, iPayload[i], i, ePayload[i], i, dPayload[i]);
      #endif
    }


    ASSERT_TRUE(same) << "decoded payload does not match input payload";
    ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0";

    // TODO: Add noise to the encoded symbols and see how many can decoder correct.

  }

}


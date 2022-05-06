/*!
 * @file qa_NoFEC.cpp
 * @author Steven Knudsen
 * @date July 15, 2021
 *
 * @details Unit test for the NoFEC class.
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

#include "NoFEC.hpp"

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

#define QA_NOFEC_DEBUG 0 // set to 1 for debugging output

/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(noFEC, Foo )
{
  /* ---------------------------------------------------------------------
   * Confirm the NoFEC object can be constructed
   * ---------------------------------------------------------------------
   */

  // First do a little CSP config work
  csp_conf_t cspConf;
  csp_conf_get_defaults(&cspConf);
  cspConf.buffer_data_size = 4096; // TODO set as CSP_MTU
  csp_init(&cspConf);

  // Set the length of the test CSP packet so it all fits into a transparent mode payload
  const unsigned long int testCSPPacketLength = 10;

  FEC * noFEC = new NoFEC(ErrorCorrection::ErrorCorrectionScheme::NO_FEC);

  ASSERT_TRUE(noFEC != NULL) << "NoFEC failed to instantiate";

  if (noFEC) {
    csp_packet_t * packet = (csp_packet_t *) csp_buffer_get(testCSPPacketLength);

    if (packet == NULL) {
      /* Could not get buffer element */
      csp_log_error("Failed to get CSP buffer");
      FAIL() << "Failed to get CSP buffer";
      return;
    }

#if QA_NOFEC_DEBUG
    printf("size of packet padding = %ld\n", sizeof(packet->padding));
    printf("size of packet length = %ld\n", sizeof(packet->length));
    printf("size of packet id = %ld\n", sizeof(packet->id));
#endif

    int cspPacketHeaderLen = sizeof(packet->padding) + sizeof(packet->length) + sizeof(packet->id);

    for (unsigned long i = 0; i < testCSPPacketLength; i++) {
      packet->data[i] = i | 0x30; // ASCII numbers
    }
    // CSP forces us to do our own bookkeeping...
    packet->length = testCSPPacketLength;

#if QA_NOFEC_DEBUG
    printf("packet length = %d\n", packet->length);
#endif

    std::vector<uint8_t> p;

    uint8_t * pptr = (uint8_t *) packet;
    for (int i = 0; i < cspPacketHeaderLen; i++) {
      p.push_back(pptr[i]);
    }
    // This is ugly, so maybe we need to rethink using PPDU_xx?
    for (unsigned long i = 0; i < testCSPPacketLength; i++) {
      p.push_back(packet->data[i]);
    }

#if QA_NOFEC_DEBUG
    // Look at the contents :-)
    for (int i = 0; i < p.size(); i++) {
      printf("p[%d] = 0x%02x\n", i, p[i]);
    }
#endif

    // @TODO maybe make these std::vector<uint8_t> ???
//    PPDU_u8 inputPayload(p);
    PPDU_u8::payload_t encodedPayload = noFEC->encode(p);

    bool same = true;
//    std::vector<uint8_t> iPayload = inputPayload.getPayload();
//    std::vector<uint8_t> ePayload = encodedPayload.getPayload();
//    for (unsigned long i = 0; i < iPayload.size(); i++) {
//      same = same & (iPayload[i] == ePayload[i]);
//    }
    for (unsigned long i = 0; i < p.size(); i++) {
      same = same & (p[i] == encodedPayload[i]);
    }

    ASSERT_TRUE(same) << "encoded payload does not match input payload";

    PPDU_u8::payload_t dPayload;
    const PPDU_u8 ecopyPayload(encodedPayload);
//    uint32_t bitErrors = noFEC->decode(encodedPayload.getPayload(), 100.0, dPayload);
    uint32_t bitErrors = noFEC->decode(encodedPayload, 100.0, dPayload);

    same = true;
    for (unsigned long i = 0; i < p.size(); i++) {
      same = same & (p[i] == dPayload[i]);
    }

    ASSERT_TRUE(same) << "decoded payload does not match input payload";
    ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0";

    // Clean up!
    csp_buffer_free(packet);
  }

}


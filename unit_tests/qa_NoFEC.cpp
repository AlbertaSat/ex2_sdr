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

  // Set the length of the test packet so it all fits into a transparent mode payload
  const unsigned long int testPacketLength = 10;

  FEC * noFEC = new NoFEC(ErrorCorrection::ErrorCorrectionScheme::NO_FEC);

  ASSERT_TRUE(noFEC != NULL) << "NoFEC failed to instantiate";

  if (noFEC) {
    std::vector<uint8_t> packet;

    for (unsigned long i = 0; i < testPacketLength; i++) {
      packet.push_back( i | 0x30 ); // ASCII numbers
    }

#if QA_NOFEC_DEBUG
    printf("packet length = %d\n", packet.size());
    // Look at the contents :-)
    for (int i = 0; i < packet.size(); i++) {acket
      printf("p[%d] = 0x%02x\n", i, packet[i]);
    }
#endif

    // @TODO maybe make these std::vector<uint8_t> ???
    std::vector<uint8_t> encodedPayload = noFEC->encode(packet);

    bool same = true;
    for (unsigned long i = 0; i < packet.size(); i++) {
      same = same & (packet[i] == encodedPayload[i]);
    }
    ASSERT_TRUE(same) << "encoded payload does not match input payload";

    std::vector<uint8_t> dPayload;
    uint32_t bitErrors = noFEC->decode(encodedPayload, 100.0, dPayload);

    same = true;
    for (unsigned long i = 0; i < packet.size(); i++) {
      same = same & (packet[i] == dPayload[i]);
    }

    ASSERT_TRUE(same) << "decoded payload does not match input payload";
    ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0";

    delete(noFEC);
  }

}


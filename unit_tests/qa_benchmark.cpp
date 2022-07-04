#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include "ConvolutionalCodecHD.hpp"
#include "mpdu.hpp"
#include "vectorTools.h"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_CC_HD_DEBUG 0 // set to 1 for debugging output

TEST(convolutional_codec_hd, r_1_2_simple_encode_decode_no_errs )
{
  /* ----------------------------------------------------------------------
   * Create rate 1/2 CCSDS codec and test encode and decode with no errors
   *
   * The codec should not care what length of message to encode and decode.
   * For the UHF radio we have a max payload of 119 bytes, but we should test
   * it, longer, and shorter messages. Might as well follow the packet
   * lengths used in MAC testing understanding that this unit test is
   * independent of the MAC fragmentation into UHF payloads...
   *
   * ----------------------------------------------------------------------
   */
  ConvolutionalCodecHD *ccHDCodec;

  try {
    ccHDCodec = new ConvolutionalCodecHD(ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2);


    // Set the packet test lengths to be a superset of what is used in
    // other unit tests, because why not
    //    uint16_t const numPackets = 6;
    //    uint16_t packetDataLengths[numPackets] = {0, 10, 103, 119, 358, 4095};
    uint16_t const numPackets = 1;
    uint16_t packetDataLength = 1024;

    std::vector<uint8_t> packet;
    for (uint16_t currentPacket = 0; currentPacket < numPackets; currentPacket++) {

      packet.resize(0);

      // Set the payload to readable ASCII
      for (unsigned long i = 0; i < packetDataLength; i++) {
        packet.push_back( (i % 79) + 0x30 ); // ASCII numbers through to ~
      }

#if QA_CC_HD_DEBUG
      printf("packet length %d (2 bytes) %02x\n", packet.size(), packet.size());
#endif
      std::vector<uint8_t> encodedPayload = ccHDCodec->encode(packet);

      // The codeword (encoded payload) is not systematic, so the message and
      // first p.size() bytes of the codeword should have differences.
      bool same = true;
      for (unsigned long i = 0; i < packet.size(); i++) {
        same = same & (packet[i] == encodedPayload[i]);
      }
      ASSERT_FALSE(same) << "encoded payload matches payload; not possible if codeword is non-systematic";


#if QA_CC_HD_DEBUG
      printf("input packet len %ld encodedPayload len %ld\n",packet.size(),encodedPayload.size());
#endif

      // Decode the encoded payload
      std::vector<uint8_t> dPayload;
      uint32_t bitErrors = ccHDCodec->decode(encodedPayload, 100.0, dPayload);

      // Convolutional decoding cannot tell how many bit errors there might be
      ASSERT_TRUE(bitErrors == 0) << "Bit error count > 0; Convolutional coding does not proivide this number...";


      // Check the decoded and original messages match
      ASSERT_TRUE(packet.size() == dPayload.size()) << "decoded payload size does not match input payload size";
#if QA_CC_HD_DEBUG
      printf("packet len %ld packet len %ld encoded len %ld decoded len %ld\n",
        packetDataLengths[currentPacket], iPayload.size(), encodedPayload.size(), dPayload.size());
#endif
      if (bitErrors == 0) {
        same = true;
        for (unsigned long i = 0; i < packet.size(); i++) {
          same = same & (packet[i] == dPayload[i]);
        }
        ASSERT_TRUE(same) << "decoded payload does not match input payload";
      }
    } // for various packet lengths

    delete ccHDCodec;

  }
  catch (FECException *e) {
    FAIL() << "Should be able to instantiate FEC for CCSDS_CONVOLUTIONAL_CODING_R_1_2.";
  }

}
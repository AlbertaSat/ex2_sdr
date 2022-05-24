/*!
 * @file qa_viterbit.cpp
 * @author Steven Knudsen
 * @date Dec 15, 2021
 *
 * @details Unit test for the viterbi class as written by vitalsong.
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

#include "viterbi.hpp"
#include "viterbi-utils.h"

using namespace std;

#include "gtest/gtest.h"

// Set this to 1 if the trellis column type is uint8_t instead of int.
// See third_party/viterbi/viterbi.hpp
#define QA_VITERBI_TRELLIS_COL_8_BIT 1

static ViterbiCodec::bitarr_t _gen_message(int num_bits)
{
  ViterbiCodec::bitarr_t msg(num_bits);
    for (int j = 0; j < num_bits; j++) {
        msg[j] = (std::rand() & 0x1);
    }
    return msg;
}

#define QA_VITERBI_DEBUG 0 // set to 1 for debugging output

/*!
 * @brief Test Main Constructors, the one that is parameterized, and the one
 * that takes the received packet as input
 */
TEST(viterbi, Poly_7x5_err )
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 3 and polynomials 0b111 and 0b101
   * ----------------------------------------------------------------------
   */
  ViterbiCodec codec(3, {7, 5});
  ASSERT_EQ(codec.decodeTruncated("001110000110011111100010110011"_b), "010111001010001"_b);

  // Inject 1 error bit.
  ASSERT_EQ(codec.decodeTruncated("001110000110011111000010110011"_b), "010111001010001"_b);
}


TEST(Viterbi, Poly_7x6_err)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 3 and polynomials 0b111 and 0b110
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(3, {7, 6});
    ASSERT_EQ(codec.decodeTruncated("101101010011"_b), "101100"_b);

    // Inject 1 error bit.
    ASSERT_EQ(codec.decodeTruncated("101101110011"_b), "101100"_b);
}

TEST(Viterbi, Poly_6x5_err)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 3 and polynomials 0b110 and 0b101
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(3, {6, 5});
    ASSERT_EQ(codec.decodeTruncated("01101101110110"_b), "1001101"_b);

#if QA_VITERBI_TRELLIS_COL_8_BIT
    // Inject 1 error bits.
    ASSERT_EQ(codec.decodeTruncated("01101101110010"_b), "1001101"_b);
#else
    // Inject 2 error bits.
    ASSERT_EQ(codec.decodeTruncated("11101101110010"_b), "1001101"_b);
#endif
}

TEST(Viterbi, Poly_91x117x121_err)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 7 and polynomials 0b1011011,
   * 0b1110101, and 0b1111001
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(7, {91, 117, 121});
    ASSERT_EQ(codec.decode("111100101110001011110101"_b), "10110111"_b);

    // Inject 4 error bits.
    ASSERT_EQ(codec.decode("100100101110001011110101"_b), "10110111"_b);
}

TEST(Viterbi, Voyager_err)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 7 and polynomials 0b1101101, and
   * 0b1001111 and 5% errors induced
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(7, {109, 79});
    auto message = _gen_message(32);
    auto encoded = codec.encode(message);

    // add 5% errors
    unsigned int nerr = encoded.size() * 0.05;
    for (size_t i = 0; i < nerr; i++) {
        int idx = rand() % encoded.size();
        encoded[idx] = (encoded[idx] == 0) ? (1) : (0);
    }

    auto decoded = codec.decodeTruncated(encoded);
    ASSERT_EQ(message, decoded);
}

// Test the given ViterbiCodec by randomly generating 10 input sequences of
// length 8, 16, 32 respectively, encode and decode them, then test if the
// decoded string is the same as the original input.
void TestViterbiCodecAutomatic(const ViterbiCodec& codec, bool truncated)
{
    for (int num_bits = 8; num_bits <= 32; num_bits <<= 1) {
        for (int i = 0; i < 10; i++) {
            auto message = _gen_message(num_bits);
            auto encoded = codec.encode(message);
            if (truncated) {
              auto decoded = codec.decodeTruncated(encoded);
#if QA_VITERBI_DEBUG
              printf("lengths: message %ld encoded %ld decoded %ld\n",
                message.size(), encoded.size(), decoded.size());
#endif
              ASSERT_EQ(decoded, message);
            }
            else {
              auto decoded = codec.decode(encoded);
#if QA_VITERBI_DEBUG
              printf("lengths: message %ld encoded %ld decoded %ld\n",
                message.size(), encoded.size(), decoded.size());
#endif
              ASSERT_EQ(decoded, message);
            }
        }
    }
}

TEST(Viterbi, Poly_7x5)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 3 and polynomials 0b111 and 0b101
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(3, {7, 5});
    TestViterbiCodecAutomatic(codec, true);
}

TEST(Viterbi, Poly_6x5)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 3 and polynomials 0b110 and 0b101
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(3, {6, 5});
    TestViterbiCodecAutomatic(codec, true);
}

TEST(Viterbi, Voyager)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 7 and polynomials 0b1101101, and
   * 0b1001111
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(7, {109, 79});
    TestViterbiCodecAutomatic(codec, true);
}

TEST(Viterbi, LTE)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 7 and polynomials 0b1011011,
   * 0b1110101, and 0b1111001
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(7, {91, 117, 121});
    TestViterbiCodecAutomatic(codec, false);
}

TEST(Viterbi, CDMA_2000)
{
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 9 and polynomials 0x1F5, 0x1B9,
   * 0x14B, and 0x13B
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(9, {501, 441, 331, 315});
    TestViterbiCodecAutomatic(codec, false);
}

#if QA_VITERBI_TRELLIS_COL_8_BIT
#else
TEST(Viterbi, Cassini)
{
    // Cassini / Mars Pathfinder
  /* ----------------------------------------------------------------------
   * Confirm codec with constraint length 15 and polynomials 0x000F, 0x4599,
   * 0x4EA5, 0x5D47, 0x76F3, 0x7EB7, and 0x695F
   * ----------------------------------------------------------------------
   */
    ViterbiCodec codec(15, {15, 17817, 20133, 23879, 30451, 32439, 26975});
    TestViterbiCodecAutomatic(codec);
}
#endif

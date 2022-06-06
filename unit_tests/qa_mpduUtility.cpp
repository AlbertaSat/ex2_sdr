/*!
 * @file mpduUtility.cpp
 * @author Steven Knudsen
 * @date May 18, 2022
 *
 * @details Unit test for the MPDU Utility class.
 *
 * @note The class has only static methods, so no constructor or other object-based
 * tests.
 *
 * @copyright AlbertaSat 2022
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <iostream>
#include <vector>

#include "mpduUtility.hpp"

using namespace std;
using namespace ex2::sdr;

#include "gtest/gtest.h"

#define QA_MPDUUTILITY_DEBUG 0 // set to 1 for debugging output

/*!
 * @brief
 */
TEST(mpduUtility, PackingUnpacking )
{
  //----------------------------------------------------------------------
  // Test Outline
  //----------------------------------------------------------------------
  // Create packed pdu
  // Unpack the pdu
  //    confirm pdu samples are valid
  // Redundantly unpack the pdu
  //    confirm pdu samples are valid
  // Pack the pdu
  //    confirm pdu samples are valid
  // Redundantly pack the pdu again
  //    confirm pdu samples are valid

  uint8_t originalData[] = {0x55,0xAA,0x00,0xFF};
  std::vector<uint8_t> pdu(originalData, originalData+4);

  unsigned long packedPDUSize = pdu.size();

  // repack to unpacked format
  MPDUUtility::repack(pdu, MPDUUtility::BPSymb_8, MPDUUtility::BPSymb_1);

  ASSERT_TRUE(pdu.size() == packedPDUSize * 8) << "Bad redundant unpacked length.";

  // check unpacked results
  uint8_t unpacked[] = {//{0x55,0xAA,0x00,0xFF};
      0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01, // 0x55
      0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00, // 0xAA
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 0x00
      0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01  // 0xFF
  };

  bool same = true;
  for (uint32_t i = 0; i < pdu.size(); i++) {
    same = same & (unpacked[i] == pdu[i]);
  }
  ASSERT_TRUE(same) << "Unpacked data doesn't match expected.";

  // repack again (redundant; already at 1 BPS)
  MPDUUtility::repack(pdu, MPDUUtility::BPSymb_1, MPDUUtility::BPSymb_1);

  ASSERT_TRUE(pdu.size() == packedPDUSize * 8) << "Bad redundant unpacked length.";

  same = true;
  for (uint32_t i = 0; i < pdu.size(); i++) {
    same = same & (unpacked[i] == pdu[i]);
  }
  ASSERT_TRUE(same) << "Redundant unpacked, 1 bit per symbol, data doesn't match expected.";


  // repack to packed format, 8 bits per symbol
  MPDUUtility::repack(pdu, MPDUUtility::BPSymb_1, MPDUUtility::BPSymb_8);

  ASSERT_TRUE(pdu.size() == packedPDUSize) << "Bad packed length.";

  same = true;
  for (uint32_t i = 0; i < pdu.size(); i++) {
    same = same & (originalData[i] == pdu[i]);
  }
  ASSERT_TRUE(same) << "Repacked data doesn't match expected.";

  // repack redundantly to packed format
  MPDUUtility::repack(pdu, MPDUUtility::BPSymb_8, MPDUUtility::BPSymb_8);

  ASSERT_TRUE(pdu.size() == packedPDUSize) << "Bad packed length.";

  same = true;
  for (uint32_t i = 0; i < pdu.size(); i++) {
    same = same & (originalData[i] == pdu[i]);
  }
  ASSERT_TRUE(same) << "Repacked data doesn't match expected.";
}

/*!
 * @brief Test that the reverse and roll operations work.
 */
TEST(mpduUtility, check_reverse_roll )
{
  //----------------------------------------------------------------------
  // Test Outline
  //----------------------------------------------------------------------
  // Create packed pdu using good time and good data vector
  //    confirm pdu is valid
  // Test byte and bit reversal
  //    confirm against handmade references
  // Test bit roll operation
  //    confirm against handmade references

//  PPDU_u8::payload_t packedData = {0x55,0xAA,0x00,0xFF};
//
//  // Create packed pdu and confirm
//  PPDU_u8 packedPPDU(packedData);
//  ASSERT_TRUE(packedPPDU.payloadLength() == 4) << "Bad packed length.";
//
//  PPDU_u8::payload_t p = packedPPDU.getPayload();
//
//  ASSERT_TRUE(p.size() == packedData.size()) << "Bad packed length.";
//
//  bool same = true;
//  for (uint32_t i = 0; i < p.size(); i++) {
//    same = same & (packedData[i] == p[i]);
//  }
//  ASSERT_TRUE(same) << "Packed data doesn't match expected.";
//
//  PPDU_u8::payload_t byteReversedData = {0xFF,0x00,0xAA,0x55};
//  PPDU_u8::payload_t bitReversedData = {0xFF,0x00,0x55,0xAA};
//
//  packedPPDU.reverse(true);
//  PPDU_u8::payload_t pr = packedPPDU.getPayload();
//
//  same = true;
//  for (uint32_t i = 0; i < p.size(); i++) {
//    same = same & (byteReversedData[i] == pr[i]);
//  }
//  ASSERT_TRUE(same) << "Reversed bytes don't match expected.";
//
//  PPDU_u8::payload_t intetrestingBitData =
//    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
//     0xF0,0xE0,0xD0,0xC0,0xB0,0xA0,0x90,0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10,0x00};
//
//  PPDU_u8::payload_t reversedInterestingData =
//    {0x00,0x08,0x04,0x0C,0x02,0x0A,0x06,0x0E,0x01,0x09,0x05,0x0D,0x03,0x0B,0x07,0x0F,
//     0xF0,0x70,0xB0,0x30,0xD0,0x50,0x90,0x10,0xE0,0x60,0xA0,0x20,0xC0,0x40,0x80,0x00};
//
//  // Create ppdu
//  PPDU_u8 interestingPPDU(intetrestingBitData);
//
//  interestingPPDU.reverse(false);
//  PPDU_u8::payload_t pbr = interestingPPDU.getPayload();
//
//  same = true;
//  for (uint32_t i = 0; i < pbr.size(); i++) {
//    same = same & (reversedInterestingData[i] == pbr[i]);
//  }
//  ASSERT_TRUE(same) << "Reversed bits don't match expected.";
//
//  PPDU_u8::payload_t oneBitData = {0x00,0x10,0x00,0x00};
//  PPDU_u8::payload_t oneBitRollRight5Data = {0x00,0x00,0x80,0x00};
//  PPDU_u8::payload_t oneBitRollLeft9Data = {0x01,0x00,0x00,0x00};
//
//  // Create ppdu
//  PPDU_u8 oneBitPPDU(oneBitData);
//
//  oneBitPPDU.roll(5,false);
//  PPDU_u8::payload_t oneBitRight = oneBitPPDU.getPayload();
//
//  same = true;
//  for (uint32_t i = 0; i < oneBitRight.size(); i++) {
//    same = same & (oneBitRollRight5Data[i] == oneBitRight[i]);
//  }
//  ASSERT_TRUE(same) << "Rolled bits don't match expected.";
//
//  oneBitPPDU.roll(9,true);
//  PPDU_u8::payload_t oneBitLeft = oneBitPPDU.getPayload();
//
//  same = true;
//  for (uint32_t i = 0; i < oneBitLeft.size(); i++) {
//    same = same & (oneBitRollLeft9Data[i] == oneBitLeft[i]);
//  }
//  ASSERT_TRUE(same) << "Rolled bits don't match expected.";
}

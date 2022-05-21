/*!
 * @file mpduUtility.cpp
 * @author Steven Knudsen
 * @date May 19, 2022
 *
 * @details MPDU utilities; see header descriptions.
 *
 * @copyright University of Alberta 2022
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "mpduUtility.hpp"

#include <algorithm>
#include <cstdlib>

namespace ex2
{
  namespace sdr
  {

    void
    MPDUUtility::repack (
      std::vector<uint8_t>& payload,
      BitsPerSymbol currentBps,
      BitsPerSymbol newBps)
    {
      // already done?
      if (currentBps == newBps) return;

      if (currentBps == BitsPerSymbol::BPSymb_8 and newBps == BitsPerSymbol::BPSymb_1)
        unpack (payload);
      else if (currentBps == BitsPerSymbol::BPSymb_1 and newBps == BitsPerSymbol::BPSymb_8)
        pack (payload);
      else
      {
        // Determine how many output bytes are needed
        unsigned int packedBitsCount = payload.size () * currentBps;
        div_t symbs = div (packedBitsCount, newBps);
        unsigned int repackedCount = symbs.quot;
        if (symbs.rem > 0) repackedCount++;

        std::vector<uint8_t> repackedPayload (repackedCount);

        uint8_t packedSymb = 0;
        uint8_t repackedSymb = 0;

        unsigned int packedSymbsProcessed = 0;
        unsigned int packedBitsProcessed = 0;
        unsigned int repackedSymbsProcessed = 0;
        unsigned int repackedBitsProcessed = 0;
        uint8_t mask;

        for (unsigned int bits = 0; bits < packedBitsCount; bits++)
        {
          repackedSymb <<= 1;

          if (packedBitsProcessed == 0) // new packed symbol
            packedSymb = payload[packedSymbsProcessed++];

          mask = currentBps - packedBitsProcessed - 1;
          repackedSymb |= (packedSymb >> mask) & 0x01;

          if (repackedBitsProcessed == (unsigned int)(newBps - 1))
          {
            repackedPayload[repackedSymbsProcessed++] = repackedSymb;
            repackedSymb = 0;
          }

          packedBitsProcessed = (packedBitsProcessed + 1) % currentBps;
          repackedBitsProcessed = (repackedBitsProcessed + 1) % newBps;
        } // for

        if (repackedSymbsProcessed < repackedCount)
        {
          repackedSymb <<= (newBps - repackedBitsProcessed);
          repackedPayload[repackedSymbsProcessed++] = repackedSymb;
        }

        payload = repackedPayload;
      }
    }

    void
    MPDUUtility::pack (std::vector<uint8_t>& payload)
    {

      // Determine how many output bytes are needed
      div_t symbs = div (payload.size (), 8);
      unsigned int packedCount = symbs.quot;
      if (symbs.rem > 0) packedCount++;

      std::vector<uint8_t> packedPayload (packedCount);

      unsigned int uIdx; // unpacked index
      unsigned int pIdx; // packed index
      unsigned int bIdx; // bit index
      uint8_t packing = 0;

      bIdx = 0;
      pIdx = 0;
      for (uIdx = 0; uIdx < payload.size (); uIdx++)
      {
        // OR the current unpacked sample into the packing var
        packing |= (payload[uIdx] & 0x01);
        bIdx++;
        if (bIdx == 8) // if packing full, save it
        {
          packedPayload[pIdx] = packing;
          pIdx++;
          packing = 0;
          bIdx = 0;
        }
        else
          packing <<= 1;
      }

      // If the last byte was not full, may need an extra shift.
      // Keep in mind that there is one last shift above if symbs.rem > 0
      if (symbs.rem > 0) packedPayload[pIdx] = packing << (7 - symbs.rem);

      payload = packedPayload;
    } // pack

    void
    MPDUUtility::unpack (std::vector<uint8_t>& payload)
    {
      // have to assume that all bits in a packed payload are required,
      // so the number of unpacked samples will always be a multiple of 8
      unsigned int unpackedCount = payload.size () * 8;
      std::vector<uint8_t> unpackedPayload (unpackedCount);

      unsigned int uIdx = 0; // unpacked index
      uint8_t packedSample;

      for (unsigned int pIdx = 0; pIdx < payload.size (); pIdx++)
      {
        packedSample = payload[pIdx];

        // might as un-roll the loop
        unpackedPayload[uIdx++] = (packedSample >> 7) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 6) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 5) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 4) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 3) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 2) & 0x01;
        unpackedPayload[uIdx++] = (packedSample >> 1) & 0x01;
        unpackedPayload[uIdx++] = packedSample & 0x01;
      }

      payload = unpackedPayload;

    } // unpack

    void
    MPDUUtility::reverse(std::vector<uint8_t>& payload, BitsPerSymbol currentBps, bool byteLevel)
    {
      BitsPerSymbol tempBPS = currentBps;
      if (!byteLevel) {
        if (tempBPS != BitsPerSymbol::BPSymb_1) {
          unpack(payload);
        }
      }
      std::reverse(payload.begin(), payload.end());
      if (!byteLevel) {
        if (tempBPS != BitsPerSymbol::BPSymb_1) {
          repack(payload, BitsPerSymbol::BPSymb_1, tempBPS);
        }
      }
    }

    void
    MPDUUtility::roll(std::vector<uint8_t>& payload, BitsPerSymbol currentBps, uint32_t numBits, bool left)
    {
      if (numBits == 0) return;
      BitsPerSymbol tempBPS = currentBps;
      repack(payload, currentBps, BitsPerSymbol::BPSymb_1);
      uint32_t shiftPositions = numBits % payload.size();
      if (left) {
        std::rotate(payload.begin(), payload.begin()+shiftPositions, payload.end());
      } else {
        std::rotate(payload.begin(), payload.end()-shiftPositions, payload.end());
      }
      repack(payload, BitsPerSymbol::BPSymb_1, tempBPS);
    }

  } /* namespace sdr */
} /* namespace ex2 */


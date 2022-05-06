/*!
 * @file QCLDPC.cpp
 * @author Sknud
 * @date Sept 27, 2021
 *
 * @details The "QCLDPC" scheme extends the FEC base class to implement the
 * IEEE_802_11N_QCLDPC_xxx forward error correction.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "QCLDPC.hpp"
#include "mpdu.hpp"

namespace ex2 {
  namespace sdr {

    QCLDPC::QCLDPC(ErrorCorrection::ErrorCorrectionScheme ecScheme) : FEC(ecScheme){
      m_errorCorrection = new ErrorCorrection(ecScheme, (MPDU::maxMTU() * 8));
    }

    QCLDPC::~QCLDPC() {
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
    }

    PPDU_u8::payload_t
    QCLDPC::encode(const PPDU_u8::payload_t &payload) {
      // @todo For now we pretend to encode the payload according to the rate
      // @todo change me!

      // Note that for some IEEE_802_11N_QCLDPC_xxx schemes, the message length
      // is not a integer number of bytes. E.g., for IEEE_802_11N_QCLDPC_648_R_1_2
      // the message length is 324 bits = 40.5 bytes. Rather than do everything
      // using 1 bit per byte (and consuming lots of memory), we choose to accept
      // payloads that are the floor of the fractional message length. That is
      // checked next.
      uint32_t messageLenBits = m_errorCorrection->getMessageLen(); // bits
      if (payload.size() != (messageLenBits / 8))
        throw FECException("QCLDPC encode payload wrong length");

      // @TODO temp code to fake a codeword. In the actual implementation,
      // we'd repack the payload to 1 bit per byte and, if needed, append zeros
      // to make it the true message length, then encode, then repack to 8 bits
      // per byte and return
      PPDU_u8::payload_t payloadData = payload;
      // extend the unencoded payload to the codeword length, padding with zeros
      payloadData.resize(m_errorCorrection->getCodewordLen()/8,0);
      // @TODO don't forget to convert to bits, or change LDPC code to work with
      // bytes as input and output
      return payloadData;
    }

    uint32_t
    QCLDPC::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate; // Not used in this method

      // @todo For now we pretend to dencode the payload according to the rate

      decodedPayload.resize(0); // Resize in all FEC decode methods

      // Here is where we apply the FEC decode algorithm.
      // For no FEC, just copy the data
      decodedPayload.insert(decodedPayload.end(),
        encodedPayload.begin(), encodedPayload.begin() + m_errorCorrection->getMessageLen()/8);

      // @TODO Consistent with the encode method, if the final message length is
      // not a multiple of 8 bits, we simply drop the last bits to make it so
      // since the encode method above originally made those zeros
      return 0;
    }

  } /* namespace sdr */
} /* namespace ex2 */

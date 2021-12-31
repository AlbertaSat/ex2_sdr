/*!
 * @file mac.cpp
 * @author Steven Knudsen
 * @date June 18, 2019
 *
 * @details
 *
 * @copyright Xiphos Systems Corp. 2019
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "mac.hpp"
#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif

#include "csp_buffer.h"

#ifdef __cplusplus
}
#endif

#include "golay.h"
#include "mpdu.hpp"
#include "QCLDPC.hpp"
#include "radio.h"

#define MAC_DEBUG 1 // Set to one to turn on debugging messages

namespace ex2 {
  namespace sdr {

#if MAC_DEBUG
  uint32_t mpdusPerCSPPacket(csp_packet_t * cspPacket, ErrorCorrection &errorCorrection) {
  // Get length of CSP packet in bytes. Make sure we are not fooled by
  // alignment, so add up the struct members
  uint32_t cspPacketSize = sizeof(csp_packet_t) + cspPacket->length;

  return MPDU::mpdusInNBytes(cspPacketSize, errorCorrection);
} // mpdusPerCSPPacket
#endif


    MAC::MAC (RF_Mode::RF_ModeNumber rfModeNumber,
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme) :
                         m_rfModeNumber(rfModeNumber)
    {
      m_updateErrorCorrection(errorCorrectionScheme);

      // @TODO This should be estimated by the system somehow and updated regularly
      m_SNREstimate = 50.0; // dB
    }

    MAC::~MAC () {
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
    }

    void
    MAC::m_updateErrorCorrection(
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme) {

      // Make a new ErrorCorrection object
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
      m_errorCorrection = new ErrorCorrection(errorCorrectionScheme, (MPDU::maxMTU() * 8));

      // Use the FEC factory to get the current FEC codec
      m_FEC = FEC::makeFECCodec(errorCorrectionScheme);

      // Always reset the first CSP fragment received flag if the FEC changes
      m_firstCSPFragmentReceived = false;

      m_currentCSPPacketLength = 0;
      m_mpduCount = 0;
      m_expectedMPDUs = 0;

      // @todo clear buffers?
      m_codewordBuffer.resize(0);
      m_transparentModePayloads.resize(0);
    }

    void
    MAC::setErrorCorrectionScheme (
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme)
    {
      // Lock things to ensure that a currently processing packet is incorrectly
      // encoded or decoded
      //        std::unique_lock<std::mutex> lck(m_ecSchemeMutex, std::defer_lock);
      //        if (lck.try_lock()) {
      //          printf("Got lock.\n");
      m_errorCorrection->setErrorCorrectionScheme(errorCorrectionScheme);
      m_updateErrorCorrection(errorCorrectionScheme);
      //        }
    }

    MAC::MAC_UHFPacketProcessingStatus
    MAC::processUHFPacket(const uint8_t *uhfPayload, const uint32_t payloadLength) {

      // Make an MPDU from the @p uhfPayload. This causes the recevied MPDUHeader
      // data to be decoded. If that fails, an exception is thrown and we can
      // short-circuit some processing
      std::vector<uint8_t> p;
      p.assign(uhfPayload, uhfPayload+payloadLength);
      try {
        MPDU mpdu(p);

        // Could do some header checks in case we are unlucky. Check packet length is >= 0, for example

        // The first MPDU transmitted will have the CSP header in it, which is
        // necessary to make the full CSP packet. Without it, there is no point
        // in making a CSP packet so we keep track of its reception.
        if (m_firstCSPFragmentReceived) {
          // If we are here, we expected more raw MPDUs.

          // We could check the CSP packet length based on the current MPDU header
          // data, but what should we do if they don't match?
#if MAC_MPDU_CHECK
          uint32_t cspPacketLenggth = mpdu.getMpduHeader()->getUserPacketPayloadLength() + sizeof(csp_packet_t);
          if (m_currentCSPPacketLength != cspPacketLenggth) {
            printf("m_currentCSPPacketLength = %d doesn't match CSP packet length %ld from current MPDU\n", m_currentCSPPacketLength, cspPacketLenggth);
          }
#endif
          // If the received raw MPDU index matches the current count, this is
          // the next raw MPDU we expected.
          if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == m_mpduCount) {
            m_expectedMPDUs = MPDU::mpdusInNBytes(m_currentCSPPacketLength, *m_errorCorrection);
            m_mpduCount++;
//            printf("expectedMPDUs = %ld m_mpduCount = %ld\n", m_expectedMPDUs,m_mpduCount);

            // append to the codewordBuffer this MPDU payload, which may contain
            // some part of a codeword or multiple codewords (remembering
            // codewords are packed into one or more consecutive MPDUs)
            m_codewordBuffer.insert(m_codewordBuffer.end(),mpdu.getCodeword().begin(),mpdu.getCodeword().end());
//            printf("after %ld raw MPDUS, m_codewordBuffer length %ld\n",m_mpduCount,m_codewordBuffer.size());
          }
          else {
            // The received raw MPDU index is not what we expected, which implies
            // that a transparent mode packet got dropped somehow, so the index
            // should be greater than what we expected. If it is, we need to
            // pad m_codewordBuffer to catch things up
            if (mpdu.getMpduHeader()->getCodewordFragmentIndex() > m_mpduCount) {
              uint32_t numMissingMPDUs = m_mpduCount - mpdu.getMpduHeader()->getCodewordFragmentIndex();
              // We need to first zero-fill the numMissingMPDUs, then insert
              // the payload from the one just received.
//              printf("padding for %ld missing mpdus\n",numMissingMPDUs);
              m_codewordBuffer.resize(m_codewordBuffer.size() + numMissingMPDUs * MPDU::maxMTU(), 0);
              m_codewordBuffer.insert(m_codewordBuffer.end(),mpdu.getCodeword().begin(),mpdu.getCodeword().end());
//              printf("after %ld raw MPDUS, m_codewordBuffer length %ld\n",m_mpduCount,m_codewordBuffer.size());
            }
            else {
              // Ack, the raw MPDUs are out of order, which should not happen
              // unless we missed several transparent mode packets. All we can
              // do is reset
              m_firstCSPFragmentReceived = false;
              m_mpduCount = 0;
              return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
            }
          }

          // Do we have enough raw MPDUs?
          // @todo refactor to avoid duplicate code
          m_expectedMPDUs = MPDU::mpdusInNBytes(m_currentCSPPacketLength, *m_errorCorrection);
          if (m_mpduCount == m_expectedMPDUs) {
            m_decodeCSPPacket();
            return MAC_UHFPacketProcessingStatus::CSP_PACKET_READY;
          } // Have all the MPDUs?

        }
        else {
//          printf("mpdu.getMpduHeader()->getCodewordFragmentIndex() %ld\n",mpdu.getMpduHeader()->getCodewordFragmentIndex());

          // Check if the first CSP packet fragment
          if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
//            printf("\ngot first packet\n");
            // Make note of the FEC scheme and check against current.
            // @todo Not sure this is a great idea since it could lead to being locked out.
            // For example, satellite is set to one FEC method, ground station tries to
            // command using a different one and never hears back...
            if (mpdu.getMpduHeader()->getErrorCorrectionScheme() != m_errorCorrection->getErrorCorrectionScheme()) {
              printf("First CSP packet fragment FEC scheme doesn't match current\n");
              // Don't update any counters and such as we are still waiting for
              // a first fragment with the right FEC scheme
              return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
            }
            else {
              // We keep in mind that the MPDU header only has the CSP payload
              // length when we set the current CSP packet length
              m_currentCSPPacketLength = mpdu.getMpduHeader()->getUserPacketPayloadLength() + sizeof(csp_packet_t);
//              printf("m_currentCSPPacketLength = %d\n", m_currentCSPPacketLength);

              m_expectedMPDUs = MPDU::mpdusInNBytes(m_currentCSPPacketLength, *m_errorCorrection);
              m_mpduCount++;
//              printf("expectedMPDUs = %ld m_mpduCount = %ld\n", m_expectedMPDUs,m_mpduCount);

              // save this MPDU payload that may contain some part of a codeword
              // or multiple codewords (remembering codewords are packed into
              // one or more consecutive MPDUs)
              m_codewordBuffer.resize(0);
              m_codewordBuffer.insert(m_codewordBuffer.end(),mpdu.getCodeword().begin(),mpdu.getCodeword().end());

              // If only one MPDU is expected for this CSP packet, decode the codeword(s) in the buffer
              if (m_mpduCount == m_expectedMPDUs) {
                m_decodeCSPPacket();
                return MAC_UHFPacketProcessingStatus::CSP_PACKET_READY;
              }
              m_firstCSPFragmentReceived = true;
              return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
            }

          }
          else {
            // If we don't receive the first MPDU for a CSP packet, we have to
            // wait for the next MPDU. If we have already received the first
            // MPDU for a CSP packet, we should not be here...
          }

          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }



      }
      catch (const MPDUHeaderException& e) {
        // @todo log this exception


        // If we already have the first fragment, then continue even if the
        // header was bad
        if (m_firstCSPFragmentReceived) {
          // we did receive a codeword  or codeword fragment, but it's junk since
          // there was a problem making the MPDU. Increment the count of received
          // MPDUs.
          m_mpduCount++;
          // Maybe this is the final expected MPDU? Better check.
          if (m_mpduCount == m_expectedMPDUs) {
            m_decodeCSPPacket();
            return MAC_UHFPacketProcessingStatus::CSP_PACKET_READY;
          } // Have all the MPDUs?
        }
        else {
          // If the first CSP packet fragment has not yet been received, then
          // we can't tell if this failed MPDU was supposed to be it, so all
          // we can do is continue to look for a first MPDU
          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }
      }

//      printf("m_mpduCount = %ld\n",m_mpduCount);

      return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
    }

    void
    MAC::m_decodeCSPPacket() {

      m_rawCSPPacket.resize(0);
      std::vector<uint8_t> codeword;
      std::vector<uint8_t> decodedMessage;
      uint32_t cwLen = m_errorCorrection->getCodewordLen()/8;
      uint32_t cwCount = m_codewordBuffer.size() / cwLen;
      for (uint32_t c = 0; c < cwCount; c++) {
        codeword.resize(0);
        codeword.insert(codeword.end(), m_codewordBuffer.begin()+c*cwLen, m_codewordBuffer.begin()+c*cwLen+cwLen);
        __attribute__((unused)) uint32_t bitErrors = m_FEC->decode(codeword, 100.0, decodedMessage);
        // @todo could log the bit errors
        m_rawCSPPacket.insert(m_rawCSPPacket.end(), decodedMessage.begin(), decodedMessage.end());
      }
      m_rawCSPPacket.resize(m_currentCSPPacketLength);
      m_firstCSPFragmentReceived = false;
      m_mpduCount = 0;
    }

    bool
    MAC::receiveCSPPacket(csp_packet_t * cspPacket) {
      // @TODO Lock the error correction scheme so that all of this
      // csp packet is processed using the same FEC scheme
      //      std::unique_lock<std::mutex> lck(m_ecSchemeMutex); // how to do this for the whole receive process?

      // A CSP packet is never all that big, so choose to first encode all
      // the codewords for the packet.
      // Then put each codeword into an MPDU, which effectivley creates the
      // transparent mode payload comprising the MPDU header followed by one or
      // more codeword fragments depending on how long the codeword for the
      // current FEC method is. Multiple MPDUs may be required per codeword.

      std::queue<PPDU_u8::payload_t> codewordFIFO;

      // Everything is done in units of bytes

      //      uint32_t const numMPDUsPerPacket = MPDU::mpdusPerCSPPacket(cspPacket, *m_errorCorrection);
      //      uint32_t const numMPDUsPerCodeword = MPDU::mpdusPerCodeword(*m_errorCorrection);
      uint16_t const cspPacketLength = sizeof(csp_packet_t) + cspPacket->length;

      // @note the message length returned by the ErrorCorrection object is
      // in bits. It may be that it's not a multiple of 8 bits (1 byte), so
      // we truncate the length and assume the encoder pads the message with
      // zeros for the missing bits
      uint32_t const messageLength = m_errorCorrection->getMessageLen() / 8;
#if MAC_DEBUG
      printf("current ECS = %d\n", (uint16_t) getErrorCorrectionScheme());
      uint32_t numMPDUsPerPacket = mpdusPerCSPPacket(cspPacket, *m_errorCorrection);
      printf("numMPDUsPerPacket %d cspPacketLength %d messageLength %d\n",numMPDUsPerPacket,cspPacketLength,messageLength);
#endif

      // A CSP packet is broken into codewords for transmission. Each codeword
      // is placed into (split across) one or more MPDUs. If the codeword does
      // not quite fill up the MPDU(s), it is zero-padded. Finally, each MPDU is
      // sent to the UHF radio for transmission in transparent mode.

      // The message buffer is eventually encoded to give the codeword
      PPDU_u8::payload_t message;

      // Set up the MPDU payload
      PPDU_u8::payload_t mpduPayload;
      mpduPayload.resize(0); // ensure it's empty
      uint32_t mpduPayloadBytesRemaining = MPDU::maxMTU();
      uint32_t mpduCount = 0;

      m_transparentModePayloads.resize(0);

      // Keep track of how much CSP packet data has been encoded
      uint32_t cspDataOffset = 0;
      uint32_t cspBytesRemaining = cspPacketLength;

      // The first chunk is special because we encode the csp_packet_t struct
      // members.

      // Insert in the message buffer the CSP padding, the length, and the id
      // @todo this could be a standalone method
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
          if (!m_FEC) {
            printf("m_FEC bad\n");
          }
          PPDU_u8 encodedChunk = m_FEC->encode(chunk);

          // Add codeword to current mpduPayload
          PPDU_u8::payload_t cw = encodedChunk.getPayload();
          uint32_t codewordBytesRemaining = cw.size(); // @TODO this is always the same, so could get only somewhere.

          // Don't assiume the mpduPayload is empty
          mpduPayloadBytesRemaining = MPDU::maxMTU() - mpduPayload.size();
          // The codeword could be really long, so make as many MPDUs as possible
          uint32_t codewordOffset = 0;
          while (codewordBytesRemaining >= mpduPayloadBytesRemaining) {
            // fill the mpduPayload
            mpduPayload.insert(mpduPayload.end(),
              cw.begin()+codewordOffset, cw.begin()+codewordOffset+mpduPayloadBytesRemaining);
            codewordOffset += mpduPayloadBytesRemaining;
            codewordBytesRemaining -= mpduPayloadBytesRemaining;

            // Now have a full MPDU payload, so make the MPDU and stash the raw payload
            MPDUHeader mpduHeader(m_rfModeNumber, *m_errorCorrection,
              mpduCount++, cspPacket->length, 0);
            // Make an MPDU
            MPDU mpdu(mpduHeader, mpduPayload);
            PPDU_u8::payload_t rawMPDU = mpdu.getRawMPDU();
            m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());

            // reset the mpduPayload
            mpduPayload.resize(0);
            mpduPayloadBytesRemaining = MPDU::maxMTU();
          }

          // Any remaining codeword bytes need to be added to the current
          // mpduPayload, which must have enough room since the loop above fell
          // through
          if (codewordBytesRemaining > 0) {
            mpduPayload.insert(mpduPayload.end(),
              cw.begin()+codewordOffset, cw.end());
            mpduPayloadBytesRemaining -= codewordBytesRemaining;
            codewordBytesRemaining = 0;
          }

          // prepare to make another message
          message.resize(0);
        }
        catch (FECException& e) { // @todo need an FEC exception that all subclasses inherit
        }

      } while (cspBytesRemaining > 0);

      // There may be an incomplete mpduPayload at this point, so make one
      // last MPDU if needed. The mpduPayload is incomplete only if more bytes
      // are needed to make a whole MPDU. Thus, there has to be fewer than a
      // whole mpduPayload's worth...
      if (mpduPayloadBytesRemaining > 0 && mpduPayloadBytesRemaining < MPDU::maxMTU()) {
        mpduPayload.resize(MPDU::maxMTU(),0); // zero-pad to length
        // Now have a full MPDU payload, so make the MPDU and stash the raw payload
        MPDUHeader mpduHeader(m_rfModeNumber, *m_errorCorrection,
          mpduCount++, cspPacket->length, 0);
        // Make an MPDU
        MPDU mpdu(mpduHeader, mpduPayload);
        PPDU_u8::payload_t rawMPDU = mpdu.getRawMPDU();
        m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());
      }

#if MAC_DEBUG
      printf("Total MPDU bytes = %ld\n", m_transparentModePayloads.size());
#endif
      return true; // @todo not yet sure when we'd return false. Only if FEC encoding fails, so check that possibility
    }

    uint8_t *
    MAC::mpduPayloadsBuffer() {
      return &m_transparentModePayloads.front();
    }

    uint32_t
    MAC::mpduPayloadsBufferLength() const {
      return m_transparentModePayloads.size();
    }

  } /* namespace sdr */
} /* namespace ex2 */


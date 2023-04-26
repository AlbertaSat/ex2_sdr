/*!
 * @file mac.cpp
 * @author Steven Knudsen
 * @date June 18, 2019
 *
 * @details
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "mac.hpp"
#include <cmath>

#include "golay.h"
#include "mpdu.hpp"
#include "QCLDPC.hpp"
#include "radio.h"

// The user packet fragmentation index is not used currently. We can use it
// to detect false positive Golay decoding since it should always be set to zero.
#define MPDU_HEADER_USER_PACKET_FRAGMENT_INDEX_DEFAULT 0

#define MAC_DEBUG 0 // Set to one to turn on debugging messages

namespace ex2 {
  namespace sdr {


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
      if (m_FEC != NULL) {
        delete m_FEC;
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
      if (m_FEC != NULL) {
        delete m_FEC;
      }
      m_FEC = FEC::makeFECCodec(errorCorrectionScheme);

      // Always reset the first packet fragment received flag if the FEC changes
      m_firstFragmentReceived = false;

      m_currentPacketLength = 0;
      m_mpduCodewordFragmentCount = 0;
      m_numExpectedMpduCodewordFragments = 0;

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

      // @todo turn this into a more explicit state machine

      // Make an MPDU from the @p uhfPayload. This causes the recevied MPDUHeader
      // data to be decoded. If that fails, an exception is thrown and we can
      // short-circuit some processing
      std::vector<uint8_t> p;
      p.assign(uhfPayload, uhfPayload+payloadLength);
      try {
        MPDU mpdu(p);

        // Since we do not use the userPacketFragmentIndex field of the MPDU
        // header (we set to 0 always), we can check it and catch cases where
        // the Golay decoder provides a false positive (i.e., when there are
        // more than 4 bit errors in 12 bits.
        //
        // When there is a false positive, we can't trust anything about the MPDU.
        //
        // If we have the first MPDU fragment already, this might be the MPDU
        // we are expecting. If it's not the last fragment, we can wait for
        // another to arrive and let the logic below pad things out. If it is
        // the last fragment, either a new packet will send a new first MPDU
        // fragement and the current one will complete, or it will timeout
        // and complete. Either way, here we do nothing, just return.
        //
        // If we don't have the first MPDU fragement already, this might be it,
        // but we can't trust it. So we ignore it.
        if (mpdu.getMpduHeader()->getUserPacketFragmentIndex() != MPDU_HEADER_USER_PACKET_FRAGMENT_INDEX_DEFAULT) {
#if MAC_DEBUG
            printf("Received an MPDU with user packet fragment index not zero\n");
#endif
          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }

        // A common reason making an MPDU will fail and throw an exception is
        // that the packet header information is corrupt. We rely on the
        // MPDUHeader class to check that, including that the error correction
        // scheme in the header is consistent with the current one in use.

        // If we already have the first MPDU, we are looking for more...
        if (m_firstFragmentReceived) {
          // If we are here, we expected more raw MPDUs.

          // We already have the user packet length and since it's in every
          // MPDU header, let's check to see if the current one is consistent
          // with the first one we received.
          if (mpdu.getMpduHeader()->getUserPacketPayloadLength() != m_currentPacketLength) {
            // Ooops, we must have missed all the MPDUs that match the first
            // MPDU fragment. This is similar to the scenario where we have the
            // first fragment, then get another first fragment. Best we can do is
            // dump the current packet processing.
#if MAC_DEBUG
            printf("Received an MPDU with user packet length that does not match the current length\n");
#endif
            m_firstFragmentReceived = false;
            m_mpduCodewordFragmentCount = 0;
            // if it happens to be the first fragment, then let's start processing for a new packet.
            if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
              m_processFirstMPDU(mpdu);

              // If only one MPDU is expected for this packet, decode the codeword(s) in the buffer
              if (m_mpduCodewordFragmentCount == m_numExpectedMpduCodewordFragments) {
                m_decodePacket();
                return MAC_UHFPacketProcessingStatus::PACKET_READY;
              }
              // Otherwise there must be more MPDUs to come...
              m_firstFragmentReceived = true;
            }
            return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
          } // MPDU user packet lengths don't match

          // If the received raw MPDU index matches the current count, this is
          // the next MPDU we expected.
          if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == m_mpduCodewordFragmentCount) {
            m_numExpectedMpduCodewordFragments = MPDU::mpdusInNBytes(m_currentPacketLength, *m_errorCorrection);
            m_mpduCodewordFragmentCount++;

            // append to the codewordBuffer this MPDU payload, which may contain
            // some part of a codeword or multiple codewords (remembering
            // codewords are packed into one or more consecutive MPDUs)
            m_codewordBuffer.insert(m_codewordBuffer.end(),mpdu.getPayload().begin(),mpdu.getPayload().end());
          }
          else {
            // The received MPDU index is not what we expected, which implies
            // that a one or more MPDUs got dropped somehow.
            //
            // There are 4 cases to consider.
            // 1) the index is 0 and somehow we have started to receive a new packet, or
            // 2) the index is greater than expected, but less than or equal the total expected, or
            // 3) the index is less than expected, or
            // 4) the index is more than the total expected
            //

            //
            // Case 1) The fragment index is 0, but we already have the first fragment
            //
            if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
              //
              // There is a current packet being built up from MPDUs, so we could
              // pad it out and decode it, but if the current MPDU is the only
              // one needed to complete a packet, we'd then have two packets in
              // hand and no way to return them both, so just ditch the current
              // packet and start a new one based on the current MPDU.
              m_firstFragmentReceived = false;
              m_mpduCodewordFragmentCount = 0;
              m_processFirstMPDU(mpdu);

              // If only one MPDU is expected for this packet, decode the codeword(s) in the buffer
              if (m_mpduCodewordFragmentCount == m_numExpectedMpduCodewordFragments) {
                m_decodePacket();
                return MAC_UHFPacketProcessingStatus::PACKET_READY;
              }
              // Otherwise there must be more MPDUs to come...
              m_firstFragmentReceived = true;
              return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
            }

            //
            // Case 2) The fragment index greater than expected, but less than the total expected
            //
            // We need to pad m_codewordBuffer to catch things up
            if ((mpdu.getMpduHeader()->getCodewordFragmentIndex() > m_mpduCodewordFragmentCount) &&
                (mpdu.getMpduHeader()->getCodewordFragmentIndex() <= m_numExpectedMpduCodewordFragments)) {
              uint32_t numMissingMPDUs = mpdu.getMpduHeader()->getCodewordFragmentIndex() - m_mpduCodewordFragmentCount;
              // We need to first zero-fill the numMissingMPDUs, then insert
              // the payload from the MPDU just received.
              m_codewordBuffer.insert(m_codewordBuffer.end(), numMissingMPDUs * MPDU::maxMTU(), 0);
              m_codewordBuffer.insert(m_codewordBuffer.end(), mpdu.getPayload().begin(), mpdu.getPayload().end());
              // Don't forget to update the count of fragments...
              m_mpduCodewordFragmentCount = mpdu.getMpduHeader()->getCodewordFragmentIndex() + 1;
            }
            else if ((mpdu.getMpduHeader()->getCodewordFragmentIndex() < m_mpduCodewordFragmentCount) ||
                (mpdu.getMpduHeader()->getCodewordFragmentIndex() > m_numExpectedMpduCodewordFragments)) {
              //
              // Case 3) The fragment index less than we expected
              //
              // Ack, the raw MPDUs are out of order, which should not happen
              // unless we also missed the first MPDU of the next packet.
              // The best we can do is zero-pad the current packet and return it.
              //
              // Case 4) The fragment index is more than the total expected
              //
              // We have a current packet in progress so we might as well
              // pad it out, decode, and return it. Clearly the current MPDU
              // can't be matched up with anything we know about, so we'll
              // just start looking for a new, first MPDU

              // Calculate the numMissingMPDUs
              uint32_t numMissingMPDUs = m_numExpectedMpduCodewordFragments - m_mpduCodewordFragmentCount;
              m_codewordBuffer.insert(m_codewordBuffer.end(), numMissingMPDUs * MPDU::maxMTU(), 0);

              m_decodePacket();
              return MAC_UHFPacketProcessingStatus::PACKET_READY;
            }

          } // MPDU fragement index is not what was expected

          // Do we have enough raw MPDUs?
          // @todo refactor to avoid duplicate code
          m_numExpectedMpduCodewordFragments = MPDU::mpdusInNBytes(m_currentPacketLength, *m_errorCorrection);
          if (m_mpduCodewordFragmentCount == m_numExpectedMpduCodewordFragments) {
            m_decodePacket();
            return MAC_UHFPacketProcessingStatus::PACKET_READY;
          } // Have all the MPDUs?

        }
        else {

          // Check if the first MPDU
          if (mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {

            m_processFirstMPDU(mpdu);

            // If only one MPDU is expected for this packet, decode the codeword(s) in the buffer
            if (m_mpduCodewordFragmentCount == m_numExpectedMpduCodewordFragments) {
              m_decodePacket();
              return MAC_UHFPacketProcessingStatus::PACKET_READY;
            }
            // Otherwise there must be more MPDUs to come...
            m_firstFragmentReceived = true;
            return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
          } // if first MPDU codeword fragment
          else { // @note empty else is so we can show the MPDU reconstruction logic
            // If we don't receive the first MPDU for a user packet, we have to
            // wait for the next MPDU. If we have already received the first
            // MPDU for a user packet, we should not be here...
          }

          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }

      } // try to make a good MPDU
      catch (const MPDUException& e) {
        // @todo log this exception
#if MAC_DEBUG
        printf("MPDU packet exception %s\n", e.what());
#endif
        // We are here because the data just received did not result in a valid
        // MPDU. That means that none of our tracking variables has been updated,
        // which means there is not much we can do if, for example, the data
        // just received was a "middle" MPDU.

        // If we already have the first fragment, then continue even if the
        // most recent raw MPDU was bad
        if (m_firstFragmentReceived) {
          // We did receive a codeword fragment, but it's junk since
          // there was a problem making the MPDU. Increment the count of received
          // MPDUs.
          m_mpduCodewordFragmentCount++;
          // Maybe this is the final expected MPDU? Better check.
          if (m_mpduCodewordFragmentCount == m_numExpectedMpduCodewordFragments) {
            // We have to pad out the current buffer of received MPDUs and
            // can't use the MPDU that was supposed to correspond to the just
            // received data.
            // Calculate the numMissingMPDUs, which should be 1, but let's be explicit.
            uint32_t numMissingMPDUs = m_numExpectedMpduCodewordFragments - (m_mpduCodewordFragmentCount - 1);
            m_codewordBuffer.insert(m_codewordBuffer.end(), numMissingMPDUs * MPDU::maxMTU(), 0);

            m_decodePacket();
            return MAC_UHFPacketProcessingStatus::PACKET_READY;
          } // Have all the MPDUs?

          // If it was not the final fragment expected, just continue...
        }

        // If the first user packet fragment has not yet been received, then
        // we can't tell if this failed MPDU was supposed to be it, so all
        // we can do is continue to look for a first MPDU; fall through

      } // catch

      return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
    } // processUHFPacket

    void
    MAC::m_processFirstMPDU(MPDU &firstMPDU) {
      // The first MPDU transmitted may have a user packet header in it, which
      // is necessary at the application layer to make the full packet.
      // Without it, there is no point in making an application packet so we
      // keep track of its reception.

      // Assume the first MPDU has correct information in its header.
      // Save it so we can keep track of the remaining MPDUs needed to
      // complete the user packet and do some error handling for things
      // like missing packets.
      //
      // @todo what if the header decoded with errors we don't catch?
      // @todo We could to more checking!? However, chances are a
      // subsequent packet will have the right information and cause one
      // of the error handling mechanisms to terminate the current packet
      // reconstruction.

      if (firstMPDU.getMpduHeader()->getErrorCorrectionScheme() != getErrorCorrectionScheme()) {
	// Use the error correction scheme specified in the header
	m_updateErrorCorrection(firstMPDU.getMpduHeader()->getErrorCorrectionScheme());
      }
      m_currentPacketLength = firstMPDU.getMpduHeader()->getUserPacketPayloadLength();
      m_numExpectedMpduCodewordFragments = MPDU::mpdusInNBytes(m_currentPacketLength, *m_errorCorrection);
      m_mpduCodewordFragmentCount = 1;

      // save this MPDU payload that may contain some part of a codeword
      // or multiple codewords (remembering codewords are packed into
      // one or more consecutive MPDUs)
      m_codewordBuffer.reserve(m_numExpectedMpduCodewordFragments*MPDU::maxMTU());
      m_codewordBuffer.assign(firstMPDU.getPayload().begin(),firstMPDU.getPayload().end());
    }

    void
    MAC::m_decodePacket() {

      m_rawPacket.resize(0);
      std::vector<uint8_t> codeword;
      std::vector<uint8_t> decodedMessage;
      uint32_t cwLen = m_errorCorrection->getCodewordLen()/8;
      uint32_t cwCount = m_codewordBuffer.size() / cwLen;
      for (uint32_t c = 0; c < cwCount; c++) {
        codeword.resize(0);
        codeword.insert(codeword.end(), m_codewordBuffer.begin()+c*cwLen, m_codewordBuffer.begin()+c*cwLen+cwLen);
        __attribute__((unused)) uint32_t bitErrors = m_FEC->decode(codeword, 100.0, decodedMessage);
        // @todo could log the bit errors
        m_rawPacket.insert(m_rawPacket.end(), decodedMessage.begin(), decodedMessage.end());
      }
      m_rawPacket.resize(m_currentPacketLength);
      m_firstFragmentReceived = false;
      m_mpduCodewordFragmentCount = 0;
    }

    bool
    MAC::receivePacket(uint8_t * packet, uint16_t len) {
      // @TODO Lock the error correction scheme so that all of this
      // packet is processed using the same FEC scheme
      //      std::unique_lock<std::mutex> lck(m_ecSchemeMutex); // how to do this for the whole receive process?

      // A packet is never all that big, so choose to first encode all
      // the codewords for the packet.
      // Then put each codeword into an MPDU, which effectivley creates the
      // transparent mode payload comprising the MPDU header followed by one or
      // more codeword fragments depending on how long the codeword for the
      // current FEC method is. Multiple MPDUs may be required per codeword.

      // Everything is done in units of bytes

        uint16_t const packetLength = len;

      // @note the message length returned by the ErrorCorrection object is
      // in bits. It may be that it's not a multiple of 8 bits (1 byte), so
      // we truncate the length and assume the encoder pads the message with
      // zeros for the missing bits
      uint32_t const messageLength = m_errorCorrection->getMessageLen() / 8;
#if MAC_DEBUG
      uint32_t const cwLen = m_errorCorrection->getCodewordLen() / 8;
      printf("current ECS = %d\n", (uint16_t) getErrorCorrectionScheme());
      printf("packetLength %d messageLength %d cwLen %d\n",packetLength,messageLength,cwLen);
#endif

      // A packet is broken into codewords for transmission. Each codeword
      // is placed into (split across) one or more MPDUs. If the codeword does
      // not quite fill up the MPDU(s), it is zero-padded. Finally, each MPDU is
      // sent to the UHF radio for transmission in transparent mode.

      // The message buffer is eventually encoded to give the codeword
      std::vector<uint8_t> message;

      // Set up the MPDU payload
      std::vector<uint8_t> mpduPayload;
      mpduPayload.resize(0); // ensure it's empty
      uint32_t mpduPayloadBytesRemaining = MPDU::maxMTU();
      uint32_t mpduCodewordFragmentCount = 0;

      m_transparentModePayloads.resize(0);

      // Keep track of how much packet data has been encoded
      uint32_t dataOffset = 0;
      uint32_t bytesRemaining = packetLength;

      message.resize(0);
      do {
        // Fill the rest of the message buffer with data; if not enough data is
        // available, use what's there and pad
        if (message.size() < messageLength) {
          // Check if we can fill the rest of the message
          if (bytesRemaining >= messageLength - message.size()) {
            // More than enough packet data remaining, so fill up the message
            uint32_t bytesToAppend = messageLength - message.size();
            message.insert(message.end(),
                           packet + dataOffset,
                           packet + dataOffset + messageLength - message.size());
            bytesRemaining -= bytesToAppend;
            dataOffset += bytesToAppend;
          }
          else {
            // Not enough packet data remaining, so put what there is in message
            message.insert(message.end(),
                           packet + dataOffset, packet + dataOffset + bytesRemaining);
            dataOffset += bytesRemaining; // @todo don't really need to update this
            bytesRemaining -= bytesRemaining;
            // Zero-pad the rest of the message
            message.resize(messageLength, 0);
          }
        }

        // Now apply the FEC encoding
        try {
          std::vector<uint8_t> cw = m_FEC->encode(message);

          // Add codeword to current mpduPayload
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

            // Now have a full MPDU payload, so make the MPDU and stash the raw
            // payload. Note, the MPDUHeader constructor cannot fail since the
            // only possible error would be from a bad ErrorCorrection, but that
            // would have been caught when m_errorCorrection was made.
            MPDUHeader *mpduHeader = new MPDUHeader(m_rfModeNumber, *m_errorCorrection,
              mpduCodewordFragmentCount++, packetLength, MPDU_HEADER_USER_PACKET_FRAGMENT_INDEX_DEFAULT);
            // Make an MPDU.
            // Just the same as for the MPDUHeader, there is no way for this
            // constructor to generate an exception because the only check that
            // could be made is for the MPDUHeader, which as noted above can't
            // fail.
            MPDU *mpdu = new MPDU(*mpduHeader, mpduPayload);
            std::vector<uint8_t> rawMPDU = mpdu->getRawMPDU();
            m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());
            delete mpdu;
            delete mpduHeader;
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
          // @note No FEC method will throw an exception for encoding at this time.
        }

      } while (bytesRemaining > 0);

      // There may be an incomplete mpduPayload at this point, so make one
      // last MPDU if needed. The mpduPayload is incomplete only if more bytes
      // are needed to make a whole MPDU. Thus, there has to be fewer than a
      // whole mpduPayload's worth...
      if (mpduPayloadBytesRemaining > 0 && mpduPayloadBytesRemaining < MPDU::maxMTU()) {
        mpduPayload.resize(MPDU::maxMTU(),0); // zero-pad to length
        // Now have a full MPDU payload, so make the MPDU and stash the raw
        // payload. Note, the MPDUHeader constructor cannot fail since the
        // only possible error would be from a bad ErrorCorrection, but that
        // would have been caught when m_errorCorrection was made.
        MPDUHeader *mpduHeader = new MPDUHeader(m_rfModeNumber, *m_errorCorrection,
          mpduCodewordFragmentCount++, packetLength, MPDU_HEADER_USER_PACKET_FRAGMENT_INDEX_DEFAULT);
        // Make an MPDU.
        // Just the same as for the MPDUHeader, there is no way for this
        // constructor to generate an exception because the only check that
        // could be made is for the MPDUHeader, which as noted above can't
        // fail.
        MPDU *mpdu = new MPDU(*mpduHeader, mpduPayload);
        std::vector<uint8_t> rawMPDU = mpdu->getRawMPDU();
        m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());
        delete mpdu;
        delete mpduHeader;
      }

#if MAC_DEBUG
      printf("Total MPDU bytes = %ld\n", m_transparentModePayloads.size());
      for (unsigned int i = 0; i < m_transparentModePayloads.size(); i++) {
        printf("tpmodePayloads[%04d] 0x%02x\n",i,m_transparentModePayloads[i]);
      }
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


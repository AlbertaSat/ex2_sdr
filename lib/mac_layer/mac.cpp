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


namespace ex2 {
  namespace sdr {

    //    MACException::MACException(const std::string& message) :
    //                       runtime_error(message) { }

    MAC* MAC::m_instance = 0;

    MAC*
    MAC::instance (RF_Mode::RF_ModeNumber rfModeNumber,
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme)
    {
      if (m_instance == 0)
      {
        m_instance = new MAC (rfModeNumber, errorCorrectionScheme);
      }
      return m_instance;
    }

    MAC*
    MAC::instance() {
      // @todo Not sure a C program can handle an exception... remove?
      if (m_instance == 0) {
        throw std::exception();
      }
      return m_instance;
    }

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

    //    /*!
    //     * @brief The task that receives UHF radio packets from the CSP server,
    //     * breaks them into chunks that can be processed and fit into transparent
    //     * packets, and passes those in order to the UHF radio.
    //     *
    //     * @details
    //     *
    //     * @todo refactor this?
    //     *
    //     * @param taskParameters
    //     */    void
    //     MAC::queueSendToUHFTask( void *taskParameters ) {
    //       MAC *me = static_cast<MAC *>(taskParameters);
    //       uint16_t currentBuffer = 0;
    //       for (;;) {
    //         // Get current buffer
    //         csp_packet_t * packet = me->m_CSPToUHFBuffers[currentBuffer];
    //
    //         // Get CSP packet
    //
    //         // This task will block provided INCLUDE_vTaskSuspend is set to 1 in
    //         // FreeRTOSConfig.h. No CPU time is used while in the blocked state
    //         xQueueReceive( me->xSendToUHFQueue, packet, portMAX_DELAY );
    //
    //         if (packet) {
    //           // Parse the packet into messages, encode, and send to UHF radio
    //
    //           // Determine how many transparent mode packet messages are in the
    //           // CSP packet. Check for a remainder and round up as needed
    //           uint16_t numMessages = packet->length / me->m_messageLength;
    //           if (packet->length % me->m_messageLength != 0)
    //             numMessages++;
    //
    //           std::vector<uint8_t> messageBuffer;
    //           uint16_t cspBytesRemaining = packet->length;
    //           uint32_t i;
    //           uint16_t bytesToCopy = me->m_messageLength;
    //           for (i = 0; i < numMessages; i++) {
    //             if (cspBytesRemaining < me->m_messageLength) {
    //               bytesToCopy = cspBytesRemaining;
    //               // make sure the end of the buffer is zeros
    //               messageBuffer.resize(me->m_messageLength, 0);
    //             }
    //
    //             // copy part of the CSP packet into the messageBuffer
    //             messageBuffer.assign(packet->data + i * me->m_messageLength,
    //               packet->data + i * me->m_messageLength + bytesToCopy);
    //
    //             cspBytesRemaining -= bytesToCopy;
    //             // TODO update MPDU parameters, encode message, make MPDU, then send via UART to radio
    //
    //           } // for all the messages in the CSP packet
    //
    //         } // packet is not null
    //
    //       } // for(ever)
    //     }
    //
    //     /*!
    //      * @brief The task that receives UHF radio packets, assembles them into CSP
    //      * packets, and sends them via a queue to the CSP server.
    //      *
    //      * @details Packets come from the UHF radio via UART. They can be ESTTC or
    //      * transparent mode packets. ESTTC packets should be valid since they are
    //      * checked using the CRC16 defined for all but transparent mode packets, but
    //      * some simple checks should be done before passing to the application layer
    //      * (such as length consistency).
    //      *
    //      * Transparent mode packets are passed along regardless of the CRC16 check
    //      * since we employ FEC; packets may have errors and still be correctable.
    //      * Transparent mode packets are always 128 bytes. The MAC header is
    //      * protected by Golay encoding and the payload by the selected FEC scheme.
    //      * Thus, there is a check for length (128 bytes) and MAC header decoding
    //      * success.
    //      *
    //      * @todo refactor this?
    //      *
    //      * @param taskParameters
    //      */
    //     void
    //     MAC::queueReceiveFromUHFTask( void *taskParameters ) {
    //       MAC *me = static_cast<MAC *>(taskParameters);
    //
    //       // TODO There needs to be a way to safely update the FEC scheme and/or
    //       // the RF mode while this task is running and processing packets. Would
    //       // be best done when we know no UART packets are arriving...
    //
    //       // TODO Let's assume no FEC scheme and default RF Mode for now.
    //       // TODO Should these be task parameters?
    //       ErrorCorrection::ErrorCorrectionScheme ecScheme = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
    //       RF_Mode::RF_ModeNumber rfMode = RF_Mode::RF_ModeNumber::RF_MODE_3;
    //
    //       // TODO use the indices and packet len to check progress and for errors?
    //       uint8_t cwFragIndex = 0;         // Nothing receive yet, so 0
    //       uint16_t userPacketLen = 0;      // Nothing receive yet, so 0
    //       uint8_t userPacketFragIndex = 0; // Nothing receive yet, so 0
    //
    //       std::vector<uint8_t> uartPacket; //
    //       std::vector<uint8_t> cspData;
    //       std::vector<uint8_t> codeword;
    //
    //       // Initialize things needed to recieve transparent mode packets from the UHF radio
    //       bool goodUHFPacket = true;
    //       uartPacket.resize(0);
    //       cspData.resize(0);
    //       codeword.resize(0);
    //
    //       for( ;; )
    //       {
    //         /*!
    //          * Check sci for bytes. If we get some, we have to assume that the radio
    //          * has passed a whole packet to us.
    //          *
    //          * In transparent mode the packet is supposed to be 128 bytes long. The
    //          * first byte is Data Field 1, the packet length. If it is not 128, then
    //          * either we have an bit error in that field, or maybe a AX.25 or ESTTC
    //          * packet. All these possibilities must be checked.
    //          */
    //         if(sciIsRxReady(sciREG2) != 0) {
    //
    //           // A new packet arrived; get all the bytes. First byte is Data Field 1
    //           uartPacket.resize(0);
    //           uint8_t data = sciReceiveByte(sciREG2);
    //           uartPacket.push_back(data);
    //
    //           // Get the rest of the bytes
    //           while(sciIsRxReady(sciREG2)) {
    //             data = sciReceiveByte(sciREG2);
    //             uartPacket.push_back(data);
    //           }
    //
    //           // Should be a complete packet in uartPacket. Get the length and check
    //           // if ESTTC, AX.25, or transparent mode packet.
    //           uint8_t packetLength = uartPacket[0];
    //
    //           // TODO Not sure we need to check for ESTTC or AX.25 packets. Arash and
    //           // Charles say no, but thew work is partially done, so for now keep.
    //           if (me->isESTTCPacket(uartPacket) || me->isAX25Packet(uartPacket)) {
    //             // Send up to the CSP server
    //             // TODO implement this!
    //
    //           }
    //           else {
    //             // Might be a transparent mode packet. Let's try to make an MPDU
    //             try {
    //               // make an MPDU, which does some checking of the data validity
    //               MPDU recdMPDU(uartPacket);
    //
    //               // Valid MPDU, so add payload to current codeword
    //               std::vector<uint8_t> cw = recdMPDU.getCodeword();
    //               codeword.insert(codeword.end(), cw.begin(), cw.end());
    //               cwFragIndex++;
    //
    //               if (codeword.size() > me->m_errorCorrection->getCodewordLen() ||
    //                   (cwFragIndex > me->m_numCodewordFragments) ||
    //                   (cwFragIndex != recdMPDU.getMpduHeader()->getCodewordFragmentIndex())) {
    //                 // TODO this is an error. We need to dump the current codeword
    //                 // and wait for a new UART packet codeword that starts at
    //                 // index = 0
    //               }
    //               else {
    //                 if (codeword.size() == me->m_errorCorrection->getCodewordLen()) {
    //
    //                   // We have the full codeword, so decode
    //                   std::vector<uint8_t> message = me->m_errorCorrection->decode(codeword);
    //
    //                   if (message.size() > 0) {
    //                     cspData.insert(cspData.end(), message.begin(), message.end());
    //
    //                     if (cspData.size() >= recdMPDU.getMpduHeader()->getUserPacketLength()) {
    //                       // TODO Shouldn't be greater than the packet length in the
    //                       // header, but maybe we send it anyway?
    //
    //                       // TODO inspect the cspData to make sure it is an actual
    //                       // CSP packet. If it is, put on the queue up to the CSP
    //                       // server
    //
    //                       // xQueuSend( ... )
    //
    //                       // Reset the buffers and counters
    //                       cspData.resize(0);
    //
    //                     }
    //                   }
    //                 }
    //               } // check for valid codeword
    //               codeword.resize(0);
    //             }
    //             catch (MPDUException& e) {
    //               // Log the error
    //               //              std::cerr << e.what() << std::endl;
    //               // TODO throw this error further?
    //               //              throw e;
    //               codeword.resize(0);
    //             }
    //
    //           }
    //
    //         }
    //
    //       }
    //
    //     } // queueSendTask

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

      // Calculate how many MPDUs needed per codeword
      m_numMPDUsPerCodeword = MPDU::mpdusPerCodeword(*m_errorCorrection);

      // Calculate how many codeword fragments are needed to send one codeword
      m_numCodewordFragments = m_errorCorrection->numCodewordFragments(MPDU::maxMTU());

      if (m_errorCorrection->getCodewordLen() % MPDUHeader::MACHeaderLength() != 0)
        m_numCodewordFragments++;
      m_messageLength = m_errorCorrection->getMessageLen();
      //      printf("setErrorCorrectionScheme\nscheme %d\n", (uint16_t) errorCorrectionScheme);
      //      printf("setErrorCorrectionScheme ErrorCorrction message length %ld\n",m_messageLength);

      // Always reset the first CSP fragment received flag if the FEC changes
      m_firstCSPFragmentReceived = false;

      m_codewordFragmentCount = 0;
      m_codewordFragmentsGood = true; // Always assume we decode them
      m_userPacketFragementCount = 0;

      // @todo clear buffers?
      m_codewordBuffer.resize(0);
      m_transparentModePayloads.resize(0);
      m_receiveCSPBuffer.resize(0);

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

      // @todo Work in Progress... rubbish

      uint32_t bitErrors = 0; // TODO member var...

      // Make an MPDU from the @p uhfPayload. this causes the recevied MPDUHeader
      // data to be decoded. If that fails, an exception is thrown and we can
      // short-circuit some processing
      std::vector<uint8_t> p;
      p.assign(uhfPayload, uhfPayload+payloadLength);
      try {
        MPDU mpdu(p);

        if (m_firstCSPFragmentReceived) {

        }
        else {
          // Check if the first CSP packet fragment
          if (mpdu.getMpduHeader()->getUserPacketFragmentIndex() == 0 &&
              mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
            m_firstCSPFragmentReceived = true;
            m_codewordFragmentsGood = m_codewordFragmentsGood && true;
            m_codewordFragmentCount++;
          }
          // Save this codeword chunk

          // Is this the only chunk needed?
          if (m_codewordFragmentCount == m_numMPDUsPerCodeword) {
            // Can try to decode
            PPDU_u8::payload_t decodedMessage;
            bitErrors = m_FEC->decode(mpdu.getCodeword(), m_SNREstimate, decodedMessage);
            //
          }

          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }



        m_codewordFragmentsGood = m_codewordFragmentsGood && true;
        m_codewordFragmentCount++;
      }
      catch (const MPDUHeaderException& e) {
        // @todo log this exception


        // If we already have the first fragment, then continue even if the
        // header was bad
        if (m_firstCSPFragmentReceived) {
          // we did receive a codeword  or codeword fragment, but it's junk since
          // there was a problem making the MPDU.
          m_codewordFragmentsGood = false;
          m_codewordFragmentCount++;
          // append a whole codeword of zeros
          m_codewordBuffer.resize(m_codewordBuffer.size() + m_errorCorrection->getCodewordLen()/8);


          // We were expecting another fragment, so add that many zero-value bytes
          m_receiveCSPBuffer.resize(m_receiveCSPBuffer.size()+m_errorCorrection->getMessageLen()/8);

          // Is this the last code
        }
        else {
          // If the first CSP packet fragment has not yet been received, then
          // we can just bail without doing anything to the received codeword (or
          // codeword fragment depending on the FEC scheme)
          return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
        }
      }

      // @todo mutex here?

      // Process the MPDU
      //      if (mpdu.getMpduHeader()->getUserPacketFragmentIndex() == 0 &&
      //          mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
      //        // this is the first user packet fragment and the first codeword fragment
      //        // of that packet fragment, so we are receiving a new CSP packet
      //
      //        // New CSP packet, empty the buffer
      //        // @todo rename this buffer?
      //        m_receiveUHFBuffer.resize(0);
      //        // New codeword, empty the buffer
      //        m_codewordBuffer.resize(0);
      //
      //        // @TODO check that the header matches the FEC we are using?
      //        // @TODO check it is long enough to have the CSP info needed
      //
      //        // append the current uhf payload to the codeword buffer
      //        m_codewordBuffer.insert(m_codewordBuffer.end(), p.begin(), p.end());
      //
      //        // @todo get the expected number of codeword and packet fragments
      //        // Should be a static function in ErrorCorrection
      //
      //        // Init fragment counters
      //        m_codewordFragmentCount = 1;
      //        m_userPacketFragementCount = 0;
      //
      //        // @TODO Update counters and check for counters complete.
      //        // Do we need only the one packet?
      //        if (codewordFragmentCount == m_numCodewordFragments) {
      //          PPDU_u8::payload_t decodedPayload;
      //          bitErrors = m_FEC->decode(mpdu.getCodeword(), 100.0, decodedPayload);
      //
      //          // @TODO check bitErrors and reject/reset if too many?
      //
      //          // @todo Arash comment : If only FEC detects more errors than it can correct. I didn't see such an option in
      //          // Phil Karn's decoder. The isPacket = true checks should suffice IMHO
      //
      //
      //        }
      //        //        m_userPacketFragementCount = 0;
      //
      //      }
      //      else {
      //
      //      }

      // @todo temp return
      return MAC_UHFPacketProcessingStatus::READY_FOR_NEXT_UHF_PACKET;
      //
      //       // TODO There needs to be a way to safely update the FEC scheme and/or
      //       // the RF mode while this task is running and processing packets. Would
      //       // be best done when we know no UART packets are arriving...
      //
      //
      //       // TODO use the indices and packet len to check progress and for errors?
      //       uint8_t cwFragIndex = 0;         // Nothing receive yet, so 0
      //       uint16_t userPacketLen = 0;      // Nothing receive yet, so 0
      //       uint8_t userPacketFragIndex = 0; // Nothing receive yet, so 0
      //
      //       std::vector<uint8_t> uartPacket; //
      //       std::vector<uint8_t> cspData;
      //       std::vector<uint8_t> codeword;
      //
      //       // Initialize things needed to recieve transparent mode packets from the UHF radio
      //       bool goodUHFPacket = true;
      //       uartPacket.resize(0);
      //       cspData.resize(0);
      //       codeword.resize(0);
      //
      //       for( ;; )
      //       {
      //         /*!
      //          * Check sci for bytes. If we get some, we have to assume that the radio
      //          * has passed a whole packet to us.
      //          *
      //          * In transparent mode the packet is supposed to be 128 bytes long. The
      //          * first byte is Data Field 1, the packet length. If it is not 128, then
      //          * either we have an bit error in that field, or maybe a AX.25 or ESTTC
      //          * packet. All these possibilities must be checked.
      //          */
      //         if(sciIsRxReady(sciREG2) != 0) {
      //
      //           // A new packet arrived; get all the bytes. First byte is Data Field 1
      //           uartPacket.resize(0);
      //           uint8_t data = sciReceiveByte(sciREG2);
      //           uartPacket.push_back(data);
      //
      //           // Get the rest of the bytes
      //           while(sciIsRxReady(sciREG2)) {
      //             data = sciReceiveByte(sciREG2);
      //             uartPacket.push_back(data);
      //           }
      //
      //           // Should be a complete packet in uartPacket. Get the length and check
      //           // if ESTTC, AX.25, or transparent mode packet.
      //           uint8_t packetLength = uartPacket[0];
      //
      //           // TODO Not sure we need to check for ESTTC or AX.25 packets. Arash and
      //           // Charles say no, but thew work is partially done, so for now keep.
      //           if (me->isESTTCPacket(uartPacket) || me->isAX25Packet(uartPacket)) {
      //             // Send up to the CSP server
      //             // TODO implement this!
      //
      //           }
      //           else {
      //             // Might be a transparent mode packet. Let's try to make an MPDU
      //             try {
      //               // make an MPDU, which does some checking of the data validity
      //               MPDU recdMPDU(uartPacket);
      //
      //               // Valid MPDU, so add payload to current codeword
      //               std::vector<uint8_t> cw = recdMPDU.getCodeword();
      //               codeword.insert(codeword.end(), cw.begin(), cw.end());
      //               cwFragIndex++;
      //
      //               if (codeword.size() > me->m_errorCorrection->getCodewordLen() ||
      //                   (cwFragIndex > me->m_numCodewordFragments) ||
      //                   (cwFragIndex != recdMPDU.getMpduHeader()->getCodewordFragmentIndex())) {
      //                 // TODO this is an error. We need to dump the current codeword
      //                 // and wait for a new UART packet codeword that starts at
      //                 // index = 0
      //               }
      //               else {
      //                 if (codeword.size() == me->m_errorCorrection->getCodewordLen()) {
      //
      //                   // We have the full codeword, so decode
      //                   std::vector<uint8_t> message = me->m_errorCorrection->decode(codeword);
      //
      //                   if (message.size() > 0) {
      //                     cspData.insert(cspData.end(), message.begin(), message.end());
      //
      //                     if (cspData.size() >= recdMPDU.getMpduHeader()->getUserPacketLength()) {
      //                       // TODO Shouldn't be greater than the packet length in the
      //                       // header, but maybe we send it anyway?
      //
      //                       // TODO inspect the cspData to make sure it is an actual
      //                       // CSP packet. If it is, put on the queue up to the CSP
      //                       // server
      //
      //                       // xQueuSend( ... )
      //
      //                       // Reset the buffers and counters
      //                       cspData.resize(0);
      //
      //                     }
      //                   }
      //                 }
      //               } // check for valid codeword
      //               codeword.resize(0);
      //             }
      //             catch (MPDUException& e) {
      //               // Log the error
      //               //              std::cerr << e.what() << std::endl;
      //               // TODO throw this error further?
      //               //              throw e;
      //               codeword.resize(0);
      //             }
      //
      //           }
      //
      //         }
      //
      //       }
      //

    }

    /*!
     * @brief Reset the processing of received UHF data.
     *
     * @details Some kind of error (dropped packet, bad packet, etc.) has
     * happened and the current CSP packet cannot be recovered. Reset the
     * processing.
     */
    //    void
    //    MAC::resetUHFProcessing() {
    //
    //    }

    /*!
     * @brief A complete CSP packet is ready to be sent up to the the CSP Server.
     *
     * @return true if there is a CSP packet, false otherwise
     */
    //    bool
    //    MAC::isCSPPacketReady() {
    //
    //      // @TODO implement function
    //      return false;
    //
    //    }

    /*!
     * @brief Accessor
     *
     * @details After the CSP packet has been retrieved, call @p resetUHFProcessing.
     *
     * @return Pointer to the completed CSP packet.
     */
    csp_packet_t *
    MAC::getCSPPacket() {

      // @TODO implement function
      return NULL;
    }

    /*!
     * @brief Return the number of MPDUs (aka transparent mode payloads) in the packet.
     *
     * @param cspPacket
     * @return The number of MPDUs (aka transparent mode payloads) in the @p cspPacket.
     */
    uint32_t
    MAC::numMPDUsInCSPPacket(csp_packet_t * cspPacket) {
      return MPDU::mpdusPerCSPPacket(cspPacket, *m_errorCorrection);
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

      uint32_t const numMPDUsPerPacket = MPDU::mpdusPerCSPPacket(cspPacket, *m_errorCorrection);
      //      uint32_t const numMPDUsPerCodeword = MPDU::mpdusPerCodeword(*m_errorCorrection);
      uint16_t const cspPacketLength = sizeof(csp_packet_t) + cspPacket->length;

      // @note the message length returned by the ErrorCorrection object is
      // in bits. It may be that it's not a multiple of 8 bits (1 byte), so
      // we truncate the length and assume the encoder pads the message with
      // zeros for the missing bits
      uint32_t const messageLength = m_errorCorrection->getMessageLen() / 8;
      printf("current ECS = %d\n", (uint16_t) getErrorCorrectionScheme());

      printf("numMPDUsPerPacket %d cspPacketLength %d messageLength %d\n",numMPDUsPerPacket,cspPacketLength,messageLength);

      // A CSP packet is broken into codewords for transmission. Each codeword
      // is placed into (split across) one or more MPDUs. If the codeword does
      // not quite fill up the MPDU(s), it is zero-padded. Finally, each MPDU is
      // sent to the UHF radio for transmission in transparent mode.

      // The message buffer is eventually encoded to give the codeword
      PPDU_u8::payload_t message;

      // Set up the MPDU payload and first MPDUHeader
      PPDU_u8::payload_t mpduPayload;
      mpduPayload.resize(0); // ensure it's empty
      uint32_t mpduPayloadBytesRemaining = MPDU::maxMTU();
      uint32_t mpduCount = 0;
      //      MPDUHeader mpduHeader(m_rfModeNumber, *m_errorCorrection,
      //        mpduCount++, cspPacketLength, 0);

      m_transparentModePayloads.resize(0);

      // Keep track of how much CSP packet data has been encoded
      uint32_t cspDataOffset = 0;
      uint32_t cspBytesRemaining = cspPacketLength;

      // The first chunk is special because we encode the csp_packet_t struct
      // members.

      // Insert in the message buffer the CSP padding, the length, and the id
      message.resize(0);
      message.insert(message.end(), cspPacket->padding, cspPacket->padding + CSP_PADDING_BYTES);
      cspBytesRemaining -= CSP_PADDING_BYTES;
      message.push_back((uint8_t) ((cspPacket->length & 0xFF00) >> 8));
      message.push_back((uint8_t) (cspPacket->length & 0x00FF));
      cspBytesRemaining -= sizeof(uint16_t);
      message.push_back((uint8_t) ((cspPacket->id.ext & 0xFF000000) >> 24));
      message.push_back((uint8_t) ((cspPacket->id.ext & 0x00FF0000) >> 16));
      message.push_back((uint8_t) ((cspPacket->id.ext & 0x0000FF00) >> 8));
      message.push_back((uint8_t) (cspPacket->id.ext & 0x000000FF));
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
          if (!m_FEC) {
            printf("m_FEC bad\n");
          }
          PPDU_u8 encodedChunk = m_FEC->encode(chunk);

          // Add codeword to current mpduPayload
          PPDU_u8::payload_t cw = encodedChunk.getPayload();
          uint32_t codewordBytesRemaining = cw.size(); // @TODO this is always the same, so could get only somewhere.

//          if (m_errorCorrection->getErrorCorrectionScheme() == ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2) {
//            printf("codewordBytesRemaining %ld \n",codewordBytesRemaining);
//          }
//          if (m_errorCorrection->getErrorCorrectionScheme() == ErrorCorrection::ErrorCorrectionScheme::NO_FEC) {
//            printf("codewordBytesRemaining %ld \n",codewordBytesRemaining);
//          }
          // Don't assiume the mpduPayload is empty
          mpduPayloadBytesRemaining = MPDU::maxMTU() - mpduPayload.size();
//printf("mpduPayloadBytesRemaining %ld\n",mpduPayloadBytesRemaining);
          // The codeword could be really long, so make as many MPDUs as possible
          uint32_t codewordOffset = 0;
          while (codewordBytesRemaining >= mpduPayloadBytesRemaining) {
            // fill the mpduPayload
            mpduPayload.insert(mpduPayload.end(),
              cw.begin()+codewordOffset, cw.begin()+codewordOffset+mpduPayloadBytesRemaining);
            codewordOffset += mpduPayloadBytesRemaining;
            codewordBytesRemaining -= mpduPayloadBytesRemaining;
            //          mpduPayloadBytesRemaining = 0;
//printf("1 codewordBytesRemaining %ld\n",codewordBytesRemaining);

            // Now have a full MPDU payload, so make the MPDU and stash the raw payload
            MPDUHeader mpduHeader(m_rfModeNumber, *m_errorCorrection,
              mpduCount++, cspPacketLength, 0);
            // Make an MPDU
            MPDU mpdu(mpduHeader, mpduPayload);
            PPDU_u8::payload_t rawMPDU = mpdu.getRawMPDU();
            m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());

            // reset the mpduPayload
            mpduPayload.resize(0);
            mpduPayloadBytesRemaining = MPDU::maxMTU();
          }
//          printf("2 codewordBytesRemaining %ld\n",codewordBytesRemaining);
//          printf("cspBytesRemaining %ld\n",cspBytesRemaining);

          // Any remaining codeword bytes need to be added to the current
          // mpduPayload, which must have enough room since the loop above fell
          // through
          if (codewordBytesRemaining > 0) {
            mpduPayload.insert(mpduPayload.end(),
              cw.begin()+codewordOffset, cw.end());
            mpduPayloadBytesRemaining -= codewordBytesRemaining;
            codewordBytesRemaining = 0;
          }


          //        codewordFIFO.push(encodedChunk.getPayload());


          // prepare to make another message
          message.resize(0);
        }
        catch (QCLDPCException& e) {

        }
      } while (cspBytesRemaining > 0);

      // There may be an incomplete mpduPayload at this point, so make one
      // last MPDU if needed.
//      printf("mpduPayloadBytesRemaining %ld\n",mpduPayloadBytesRemaining);
      if (mpduPayloadBytesRemaining > 0 && mpduPayloadBytesRemaining < MPDU::maxMTU()) {
        mpduPayload.resize(MPDU::maxMTU(),0); // zero-pad to length
        // Now have a full MPDU payload, so make the MPDU and stash the raw payload
        MPDUHeader mpduHeader(m_rfModeNumber, *m_errorCorrection,
          mpduCount++, cspPacketLength, 0);
        // Make an MPDU
        MPDU mpdu(mpduHeader, mpduPayload);
        PPDU_u8::payload_t rawMPDU = mpdu.getRawMPDU();
        m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());
      }

      //      printf("CSP packet of cspPacketLength %d bytes needed %ld codewords\n",cspPacketLength, codewordFIFO.size());

      // At this point we have a FIFO of codewords and need to make MPDUs so we
      // can send

      //      printf("numMPDUsPerCodeword %ld\n",numMPDUsPerCodeword);

      //Reset the fragmentation indices
      //      uint32_t codewordFragmentCount = 0;
      //      uint32_t userPacketFragementCount = 0;
      //      m_transparentModePayloads.resize(0);


      // @todo use accessor to update header indices
      //      MPDUHeader mpduHeader(UHF_TRANSPARENT_MODE_PACKET_LENGTH,
      //        m_rfModeNumber,
      //        *m_errorCorrection,
      //        codewordFragmentCount, cspPacketLength, userPacketFragementCount);

      //      while (codewordFIFO.size() > 0) {

      //
      //      printf("header check\t cw frag index %d \n", mpduHeader.getCodewordFragmentIndex());
      //      printf("header check\t cw length %d \n", mpduHeader.getCodewordLength() / 8);
      //      printf("header check\t error CS %d \n", mpduHeader.getErrorCorrectionScheme());
      //      printf("header check\t message len %d \n", mpduHeader.getMessageLength() / 8);
      //      printf("header check\t message buffer len %d \n", message.size());
      //      printf("header check\t rf mode num %d \n", mpduHeader.getRfModeNumber());
      //      printf("header check\t UHF packet len %d \n", mpduHeader.getUhfPacketLength());
      //      printf("header check\t user packet frag %d \n", mpduHeader.getUserPacketFragmentIndex());
      //      printf("header check\t user packet len %d \n", mpduHeader.getUserPacketLength());
      //
      //        PPDU_u8::payload_t  codeword = codewordFIFO.front();
      //        codewordFIFO.pop();

      //        printf("cw = %ld of %ld codeword size = %ld\n",cw, codewordFIFO.size(), codeword.size());

      //        uint32_t codewordBytesRemaining = codeword.size(); // @TODO this is always the same, so could get only once above.
      //        uint32_t codewordBytesOffset = 0;
      //        codewordFragmentCount = 0;
      //
      //        while (codewordBytesRemaining > 0) {
      //
      //          mpduPayload.resize(0);
      //
      //          if (codewordBytesRemaining >= MPDU::maxMTU()) {
      //            // No padding needed
      //            mpduPayload.insert(mpduPayload.end(),
      //              codeword.begin() + codewordBytesOffset,
      //              codeword.begin() + codewordBytesOffset + MPDU::maxMTU());
      //            codewordBytesRemaining -= MPDU::maxMTU();
      //            codewordBytesOffset += MPDU::maxMTU();
      //          }
      //          else {
      //            // Padding needed
      //            mpduPayload.insert(mpduPayload.end(),
      //              codeword.begin() + codewordBytesOffset,
      //              codeword.begin() + codewordBytesOffset + codewordBytesRemaining);
      //            codewordBytesRemaining -= codewordBytesRemaining;
      //            mpduPayload.resize(MPDU::maxMTU(), 0);
      //          }
      //
      ////          printf("codewordBytesRemaining = %ld\n",codewordBytesRemaining);
      //
      //          // Make the MPDU header. Have to do this each time because the
      //          // header data gets Golay encoded.
      //          MPDUHeader mpduHeader(/*UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH,*/
      //            m_rfModeNumber,
      //            *m_errorCorrection,
      //            codewordFragmentCount++, cspPacketLength, userPacketFragementCount);
      //          // Make an MPDU
      //          MPDU mpdu(mpduHeader, mpduPayload);
      //          PPDU_u8::payload_t rawMPDU = mpdu.getRawMPDU();
      //          m_transparentModePayloads.insert(m_transparentModePayloads.end(), rawMPDU.begin(), rawMPDU.end());
      //
      //        } // while codeword bytes remaining
      //
      //        userPacketFragementCount++;
      //
      //      } // for all the codewords in this CSP packet

      //      printf("Total transparent mode packets = %d\n", userPacketFragementCount);
      printf("Total MPDU bytes = %ld\n", m_transparentModePayloads.size());

      //      printf("There are %ld transparent mode packets ready to go...\n",transparentModePayloadFIFO.size());

      return true;
    }

    const uint32_t
    MAC::mpduPayloadLength() {
      return MPDU::maxMTU();
    }


    const uint8_t *
    MAC::mpduPayloadsBuffer() {
      return &m_transparentModePayloads.front();
    }

    const uint32_t
    MAC::mpduPayloadsBufferLength() {
      return m_transparentModePayloads.size();
    }


    //    bool
    //    MAC::nextMPDU(MPDU &mpdu){
    //
    //      // @TODO implement function
    //      return false;
    //    }


    //    bool
    //    MAC::isESTTCPacket(std::vector<uint8_t> &packet) {
    //      bool isPacket = false;
    //      // First byte should be the Data Field 1, the Data Field 2 length in bytes
    //      // Thus, the length of packet should be the value of the first byte + 1
    //
    //      if ((uint32_t)(packet[0] + 1) == packet.size()) {
    //        if ((packet[1] == 'E') &&
    //            (packet[2] == 'S') &&
    //            (packet[3] == '+')) {
    //          isPacket = true;
    //        }
    //        if ((packet[1] == 'O') &&
    //            (packet[2] == 'K')) {
    //          isPacket = true;
    //        }
    //        if ((packet[1] == '+') &&
    //            (packet[2] == 'E') &&
    //            (packet[3] == 'S')) {
    //          isPacket = true;
    //        }
    //        if ((packet[1] == 'E') &&
    //            (packet[2] == 'R') &&
    //            (packet[3] == 'R')) {
    //          isPacket = true;
    //        }
    //      } // packet length is good
    //
    //      return isPacket;
    //
    //    } // isESTTCPacket

    //    void
    //    MAC::m_clearFIFO(std::queue<PPDU_u8::payload_t> &fifo) {
    //      std::queue<PPDU_u8::payload_t> empty;
    //      std::swap(fifo, empty);
    //    }

    //    bool
    //    MAC::isAX25Packet(std::vector<uint8_t> &packet) {
    //      bool isPacket = false;
    //      // First byte should be the Data Field 1, the Data Field 2 length in bytes
    //      // Thus, the length of packet should be the value of the first byte + 1
    //
    //      if ((uint32_t)(packet[0] + 1) == packet.size()) {
    //        // The AX.25 frame starts with 9 bytes that have the value 0x7E
    //        uint8_t i;
    //        isPacket = true;
    //        for (i = 1; i <= 9; i++) {
    //          isPacket = isPacket & (packet[i] == 0x7E);
    //        }
    //      } // packet length is good
    //
    //      return isPacket;
    //
    //    } // isAX25Packet

  } /* namespace sdr */
} /* namespace ex2 */

